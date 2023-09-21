#include <Tinyfont.h>
#include <FixedPoints.h>
#include <Arduboy2.h>

#define RCVIEWWIDTH 100

// Libs for raycasting
#include <ArduboyRaycast.h>
#include <ArduboyRaycast_Map.h>
#include <ArduboyRaycast_Extra.h>

#include "mazegen.h"
#include "behaviors.h"

// Graphics
#include "resources/raycastbg.h"
#include "spritesheet.h"
#include "tilesheet.h"

//ARDUBOY_NO_USB

Arduboy2Base arduboy;
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

// And now some debug stuff
// #define DRAWMAPDEBUG         // Display map (will take up portion of screen)
// #define NOSPRITES            // Remove all sprites
// #define NOFLOOR
 #define ADDDEBUGAREA     // Add a little debug area
//  #define PRINTSPRITEDATA  // Having trouble with sprites sometimes

// Gameplay constants
constexpr uint8_t FRAMERATE = 35;
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;

// Size limit for data structures
constexpr uint8_t NUMSPRITES = 20;
constexpr uint8_t NUMBOUNDS = 16;
constexpr uint8_t MAPWIDTH = 16;
constexpr uint8_t MAPHEIGHT = 16;


//Menu related stuff
uint8_t menuIndex = 0;
uint8_t mazeSize = 1;
uint8_t mazeType = 0;

uint8_t curWidth = 0;
uint8_t curHeight = 0;

constexpr uint8_t MAZETYPECOUNT = 2;
constexpr MazeType MAZETYPES[MAZETYPECOUNT] PROGMEM = {
    { "MAZ", &genMazeType },
    { "EPT", &genSparseRandom },
};

constexpr uint8_t MAZESIZECOUNT = 2;
constexpr MazeSize MAZESIZES[MAZESIZECOUNT] PROGMEM = {
    { "SML", 11, 11 },  //NOTE: must always be 2N + 1
    { "MED", 15, 15 },
};


RcPlayer player;
RaycastInstance instance;

//Big data!
uint8_t mapBuffer[MAPHEIGHT * MAPWIDTH];
RcSprite spritesBuffer[NUMSPRITES];
RcBounds boundsBuffer[NUMBOUNDS];
SSprite tempcontainer[NUMSPRITES];

RcMap worldMap {
    mapBuffer,
    MAPWIDTH,
    MAPHEIGHT
};

RcSpriteGroup sprites {
    spritesBuffer,
    tempcontainer,
    NUMSPRITES,
    boundsBuffer,
    NUMBOUNDS
};



//setLightIntensity(&rcstate, (uflot)1.5);

//RaycastState rcstate {};
//setLightIntensity(&rcstate, (uflot)1.5);
//setLightIntensity(&rcstate, 1.5);

//rcstate.tilesheet = tilesheet;
//rcstate.spritesheet = spritesheet;
//rcstate.spritesheet_mask = spritesheet_Mask;


//Draw the floor underneath the raycast walls (ultra simple for now to save cycles)
void raycastFoundation()
{
    // Actually changed it to a full bg
    //Sprites::drawOverwrite(0, 0, raycastBg, 0);
    //raycastFloor();
}

//Simple redirection for movement attempt
bool solidChecker(uint8_t x, uint8_t y) { 
    return worldMap.getCell(x, y) & 1; 
}

bool inExit() { 
    return worldMap.getCell((int)player.posX, (int)player.posY) == TILEEXIT; 
}


// Perform ONLY player movement updates! No drawing!
void movement()
{
    float movement = 0;
    float rotation = 0;

    // move forward if no wall in front of you
    if (arduboy.pressed(UP_BUTTON))
        movement = MOVESPEED;
    if (arduboy.pressed(DOWN_BUTTON))
        movement = -MOVESPEED;

    if (arduboy.pressed(RIGHT_BUTTON))
        rotation = -ROTSPEED;
    if (arduboy.pressed(LEFT_BUTTON))
        rotation = ROTSPEED;

    tryMovement(&player, &sprites, movement, rotation, &solidChecker);
}

//Menu functionality, move the cursor, select things (redraws automatically)
void doMenu()
{
    constexpr uint8_t MENUITEMS = 3;
    int8_t menuMod = 0;
    int8_t selectMod = 0;

    if(arduboy.pressed(A_BUTTON) && arduboy.justPressed(UP_BUTTON))
        menuMod = -1;
    if(arduboy.pressed(A_BUTTON) && arduboy.justPressed(DOWN_BUTTON))
        menuMod = 1;

    menumod(menuIndex, menuMod, MENUITEMS);

    if(arduboy.justPressed(B_BUTTON))
    {
        selectMod = 1;
        switch (menuIndex)
        {
            case 0: menumod(mazeSize, selectMod, MAZESIZECOUNT); break;
            case 1: menumod(mazeType, selectMod, MAZETYPECOUNT); break;
            case 2: generateMaze(); break;
        }
    }

    // We check released in case the user was showing a hint
    if(menuMod || selectMod || arduboy.pressed(B_BUTTON) || arduboy.justReleased(B_BUTTON))
        drawMenu(arduboy.pressed(B_BUTTON) && menuIndex == 3);
}

// Draw just the menu section, does not overwrite the raycast area
void drawMenu(bool showHint)
{
    constexpr uint8_t MENUX = 105;
    constexpr uint8_t MENUY = 22;
    constexpr uint8_t MENUSPACING = 6;

    fastClear(&arduboy, VIEWWIDTH, 0, WIDTH,HEIGHT);
    FASTRECT(arduboy, VIEWWIDTH + 1, 0, WIDTH - 1, HEIGHT - 1, WHITE);
    arduboy.drawPixel(VIEWWIDTH + 3, 2, WHITE);
    arduboy.drawPixel(WIDTH - 3, 2, WHITE);
    arduboy.drawPixel(VIEWWIDTH + 3, HEIGHT - 3, WHITE);
    arduboy.drawPixel(WIDTH - 3, HEIGHT - 3, WHITE);

    tinyfont.setCursor(109, 4);
    tinyfont.print(F("3D"));
    tinyfont.setCursor(105, 9);
    tinyfont.print(F("MAZE"));

    MazeSize mzs = getMazeSize(MAZESIZES, mazeSize);
    tinyfont.setCursor(MENUX + 4, MENUY);
    tinyfont.print(mzs.name);

    MazeType mzt = getMazeType(MAZETYPES, mazeType);
    tinyfont.setCursor(MENUX + 4, MENUY + MENUSPACING);
    tinyfont.print(mzt.name);

    tinyfont.setCursor(MENUX + 4, MENUY + MENUSPACING * 2);
    tinyfont.print(F("NEW"));

    tinyfont.setCursor(MENUX, MENUY + menuIndex * MENUSPACING);
    tinyfont.print("o");
}

// Generate a new maze and reset the game to an initial playable state
void generateMaze()
{
    clearRaycast(&arduboy);
    sprites.resetAll();
    tinyfont.setCursor(12, 28);
    tinyfont.print(F("Generating maze"));
    arduboy.display();

    MazeSize mzs = getMazeSize(MAZESIZES, mazeSize);
    MazeType mzt = getMazeType(MAZETYPES, mazeType); 

    //Call the generator function chosen by the menu
    mzt.func(&worldMap, mzs.width, mzs.height, &player.posX, &player.posY, &player.dirX, &player.dirY);
    curWidth = mzs.width;
    curHeight = mzs.height;

    #ifdef ADDDEBUGAREA
    worldMap.setCell(5, 0, TILEDOOR);
    sprites.addSprite(4.5, 1.4, SPRITEBARREL, 1, 8, NULL);
    sprites.addSprite(6.5, 1.4, SPRITEBARREL, 1, 8, NULL);
    sprites.addSprite(7, 5, SPRITECHEST, 1, 8, NULL);
    sprites.addSprite(4, 3, SPRITEMONSTER, 1, 0, behavior_bat);
    RcSprite * sp = sprites.addSprite(6, 3, SPRITELEVER, 1, 8, behavior_animate_16);
    sp->intstate[0] = SPRITELEVER;
    sp->intstate[1] = 2;
    for(uint8_t y = 1; y < 8; y++)
        for(uint8_t x = 3; x < 8; x++)
            worldMap.setCell(x, y, TILEEMPTY);
    #endif
}


void setup()
{
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.flashlight();
    arduboy.initRandomSeed();
    arduboy.setFrameRate(FRAMERATE);
    instance.setLightIntensity(1.5);
    instance.tilesheet = tilesheet;
    instance.spritesheet = spritesheet;
    instance.spritesheet_mask = spritesheet_Mask;
    generateMaze();
    drawMenu(false);
}

void loop()
{
    if (!arduboy.nextFrame()) return;

    arduboy.pollButtons();

    doMenu();

    // Funny game no state variable haha
    if(inExit()) 
    {
        clearRaycast(&arduboy);

        constexpr uint8_t WINX = 22;
        constexpr uint8_t WINY = 32;

        tinyfont.setCursor(WINX, 24);
        tinyfont.print(F("COMPLETE!"));
        tinyfont.setCursor(WINX + 8, WINY);
    }
    else
    {
        #ifdef NOFLOOR
        clearRaycast(&arduboy);
        #else
        raycastFoundation();
        #endif

        instance.raycastWalls(&player, &worldMap, &arduboy);
        #ifndef NOSPRITES
        sprites.runSprites(&arduboy);
        instance.drawSprites(&player, &sprites, &arduboy); //spritesheet, spritesheet_Mask);
        #endif
        movement();

        #ifdef DRAWMAPDEBUG
        drawMaze(&arduboy, worldMap, 0, 0);
        #endif
    }

    arduboy.display();
}
