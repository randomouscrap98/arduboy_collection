#include <Tinyfont.h>
#include <FixedPoints.h>
#include <Arduboy2.h>

// #define RCSMALLLOOPS

// Libs for raycasting
#include <ArduboyRaycast.h>

// Graphics
#include "resources/raycastbg.h"
#include "spritesheet.h"
#include "tilesheet.h"

//ARDUBOY_NO_USB

Arduboy2Base arduboy;
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint8_t FRAMERATE = 45;
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;

// Since we're using this number so many times in template types, might 
// as well make it a constant.
constexpr uint8_t NUMINTERNALBYTES = 1;
constexpr uint8_t NUMSPRITES = 16;

// Once again we pick 16 sprites just in case we need them. 16 is a decent number
// to not take up all the memory but still have enough to work with.
RcContainer<NUMSPRITES, NUMINTERNALBYTES, 100, HEIGHT> raycast(tilesheet, spritesheet, spritesheet_Mask);

//// Size limit for data structures
//constexpr uint8_t NUMSPRITES = 20;
//constexpr uint8_t NUMBOUNDS = 16;
//constexpr uint8_t MAPWIDTH = 15;
//constexpr uint8_t MAPHEIGHT = 15;

//RcPlayer player;
//RcRender<100,HEIGHT> instance;
//
////Big data!
//uint8_t mapBuffer[MAPHEIGHT * MAPWIDTH];
//RcSprite<2> spritesBuffer[NUMSPRITES];
//SSprite<2> sortedSprites[NUMSPRITES];
//RcBounds boundsBuffer[NUMBOUNDS];
//
//RcMap worldMap {
//    mapBuffer,
//    MAPWIDTH,
//    MAPHEIGHT
//};
//
//RcSpriteGroup<2> sprites {
//    spritesBuffer,
//    sortedSprites,
//    boundsBuffer,
//    NUMSPRITES,
//    NUMBOUNDS
//};

////Draw the floor underneath the raycast walls (ultra simple for now to save cycles)
//void raycastFoundation()
//{
//    // Actually changed it to a full bg
//    raycast.render.drawRaycastBackground(&arduboy, raycastBg);
//}

////Simple redirection for movement attempt
//bool solidChecker(uflot x, uflot y) { 
//    return raycast.worldMap.getCell(x.getInteger(), y.getInteger()) & 1 ||
//        raycast.sprites.firstColliding(x, y, RBSTATESOLID) != NULL; 
//}

bool isSolid(uflot x, uflot y)
{
    // The location is solid if the map cell is nonzero OR if we're colliding with
    // any (solid) bounding boxes
    uint8_t tile = raycast.worldMap.getCell(x.getInteger(), y.getInteger());

    //return (tile != 0 && tile != MyTiles::OutdoorRockOpening) || 
    //    raycast.sprites.firstColliding(x, y, RBSTATESOLID) != NULL;
    return (tile != 0) || raycast.sprites.firstColliding(x, y, RBSTATESOLID) != NULL;
}

//bool inExit() { 
//    return raycast.worldMap.getCell((int)raycast.player.posX, (int)player.posY) == TILEEXIT; 
//}


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

    raycast.player.tryMovement(movement, rotation, &isSolid);
}

//Menu functionality, move the cursor, select things (redraws automatically)
/*void doMenu()
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
}*/

// Draw just the menu section, does not overwrite the raycast area
void drawMenu(bool showHint)
{
    constexpr uint8_t MENUX = 105;
    constexpr uint8_t MENUY = 22;
    constexpr uint8_t MENUSPACING = 6;

    fastClear(&arduboy, raycast.render.VIEWWIDTH, 0, WIDTH,HEIGHT);
    FASTRECT(arduboy, raycast.render.VIEWWIDTH + 1, 0, WIDTH - 1, HEIGHT - 1, WHITE);
    arduboy.drawPixel(raycast.render.VIEWWIDTH + 3, 2, WHITE);
    arduboy.drawPixel(WIDTH - 3, 2, WHITE);
    arduboy.drawPixel(raycast.render.VIEWWIDTH + 3, HEIGHT - 3, WHITE);
    arduboy.drawPixel(WIDTH - 3, HEIGHT - 3, WHITE);

    /*
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
    */
}

/*
void behavior_bat(RcSprite<2> * sprite)
{
    sprite->x = 4 + cos((float)arduboy.frameCount / 4) / 2;
    sprite->y = 3 + sin((float)arduboy.frameCount / 4) / 2;
    sprite->state = (sprite->state & ~(RSSTATEYOFFSET)) | ((16 | uint8_t(15 * abs(sin((float)arduboy.frameCount / 11)))) << 3);
}

void behavior_animate_16(RcSprite<2> * sprite)
{
    sprite->frame = sprite->intstate[0] + ((arduboy.frameCount >> 4) & (sprite->intstate[1] - 1));
}
*/

/*
// Generate a new maze and reset the game to an initial playable state
void generateMaze()
{
    instance.clearRaycast(&arduboy);
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
    sprites.addSprite(4.5, 1.4, SPRITEBARREL, 2, 8, NULL);
    sprites.addSprite(6.5, 1.4, SPRITEBARREL, 2, 8, NULL);
    sprites.addSprite(7, 5, SPRITECHEST, 2, 8, NULL);
    sprites.addSprite(4, 3, SPRITEMONSTER, 2, 0, behavior_bat);
    RcSprite<2> * sp = sprites.addSprite(6, 3, SPRITELEVER, 2, 8, behavior_animate_16);
    sp->intstate[0] = SPRITELEVER;
    sp->intstate[1] = 2;
    for(uint8_t y = 1; y < 8; y++)
    {
        for(uint8_t x = 3; x < 8; x++)
        {
            worldMap.setCell(x, y, TILEEMPTY);

            #ifdef EXTENDEDSPRITES
            if(y >= 5)
            {
                sprites.addSprite(x + 0.5, y + 0.5, 8, 1, 0, NULL);
            }
            #endif
        }
    }
    #endif

    instance.clearRaycast(&arduboy);
    tinyfont.setCursor(5, 28);
    tinyfont.print(F("Maze generation complete!"));
    arduboy.display();
}*/


void setup()
{
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.flashlight();
    arduboy.initRandomSeed();
    arduboy.setFrameRate(FRAMERATE);
    raycast.render.setLightIntensity(1.5);

    for(int i = 0; i < RCMAXMAPDIMENSION; i++)
    {
        raycast.worldMap.setCell(i, 0, 1);
        raycast.worldMap.setCell(i, RCMAXMAPDIMENSION - 1, 1);
        raycast.worldMap.setCell(0, i, 1);
        raycast.worldMap.setCell(RCMAXMAPDIMENSION - 1, i, 1);
    }
    //instance.altWallShading = RcShadingType::White;
    //instance.cornershading = 1;
    //instance.shading = RcShadingType::None;
    //instance.tilesheet = tilesheet;
    //instance.spritesheet = spritesheet;
    //instance.spritesheet_mask = spritesheet_Mask;
    //generateMaze();
    //drawMenu(false);
}

void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process player movement + interaction
    movement();
    //testAreaTransition();

    // Draw the correct background for the area. 
    drawMenu(false);
    raycast.render.drawRaycastBackground(&arduboy, raycastBg);

    raycast.runIteration(&arduboy);

    arduboy.display();

/*
    if (!arduboy.nextFrame()) return;

    arduboy.pollButtons();

    //doMenu();

    //// Funny game no state variable haha
    //if(inExit()) 
    //{
    //    instance.clearRaycast(&arduboy);

    //    constexpr uint8_t WINX = 22;
    //    constexpr uint8_t WINY = 32;

    //    tinyfont.setCursor(WINX, 24);
    //    tinyfont.print(F("COMPLETE!"));
    //    tinyfont.setCursor(WINX + 8, WINY);
    //}
    //else
    //{
        #ifdef NOFLOOR
        instance.clearRaycast(&arduboy);
        #else
        raycastFoundation();
        #endif

        instance.raycastWalls(&player, &worldMap, &arduboy);
        #ifndef NOSPRITES
        sprites.runSprites();
        instance.drawSprites(&player, &sprites, &arduboy); //spritesheet, spritesheet_Mask);
        #endif
        movement();

    //}

    arduboy.display();
    */
}
