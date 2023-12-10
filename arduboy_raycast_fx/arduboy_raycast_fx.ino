#include <Tinyfont.h>
#include <FixedPoints.h>
#include <Arduboy2.h>

#include <ArduboyFX.h>      // required to access the FX external flash
#include "fx/fxdata.h"  // this file contains all references to FX data

//#define RCSMALLLOOPS

// Libs for raycasting
#include <ArduboyRaycast.h>

// Graphics
#include "resources/raycastbg.h"
#include "spritesheet.h"
#include "tilesheet.h"

// ARDUBOY_NO_USB

constexpr uint8_t staticmap_width = 64;
constexpr uint8_t staticmap_height = 64;

Arduboy2Base arduboy;
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint8_t FRAMERATE = 45;
constexpr float MOVESPEED = 1.25f / FRAMERATE;
constexpr float ROTSPEED = 1.5f / FRAMERATE;
constexpr float RUNMULTIPLIER = 1.75;

// Since we're using this number so many times in template types, might 
// as well make it a constant.
constexpr uint8_t NUMINTERNALBYTES = 1;
constexpr uint8_t NUMSPRITES = 32;

// Some stuff for external map loading
constexpr uint8_t CAGEX = 7;
constexpr uint8_t CAGEY = 7;
constexpr uint8_t LOADWIDTH = 15;
constexpr uint8_t LOADHEIGHT = 15;

constexpr uint8_t SPRITEVIEW = 5;

//Sprites outside this radius get banished, sprites that enter this radius
//get loaded
constexpr uint8_t SPRITEBEGIN_X = CAGEX - SPRITEVIEW;
constexpr uint8_t SPRITEEND_X = CAGEX + SPRITEVIEW;
constexpr uint8_t SPRITEBEGIN_Y = CAGEY - SPRITEVIEW;
constexpr uint8_t SPRITEEND_Y = CAGEY + SPRITEVIEW;

constexpr int16_t SPRINTMIN_SECS = 1;
constexpr int16_t SPRINT_SECS = 5;
constexpr int16_t SPRINTREC_SECS = 15;
constexpr int16_t SPRINTMAX = FRAMERATE * SPRINTMIN_SECS * SPRINT_SECS * SPRINTREC_SECS; //Realistically, this should just be the lcm of all the literals below
constexpr int16_t SPRINTMIN = SPRINTMAX / (FRAMERATE * SPRINTMIN_SECS); //If you let go of sprint, you won't be able to sprint again until there's this much sprint available
constexpr int16_t SPRINTDRAIN = SPRINTMAX / (FRAMERATE * SPRINT_SECS); 
constexpr int16_t SPRINTRECOVER = SPRINTMAX / (FRAMERATE * SPRINTREC_SECS); 
//constexpr uint8_t SPRITEGC_PERFRAME = 3;  // How many sprites to loop through per frame for garbage collect

uint8_t world_x = CAGEX + 1;
uint8_t world_y = CAGEY;

int16_t sprintmeter = SPRINTMAX;
bool holding_b = false;
//uint8_t spritegc_i = 0;

// Once again we pick 16 sprites just in case we need them. 16 is a decent number
// to not take up all the memory but still have enough to work with.
RcContainer<NUMSPRITES, NUMINTERNALBYTES, 100, HEIGHT> raycast(tilesheet, spritesheet, spritesheet_Mask);

bool isSolid(uflot x, uflot y)
{
    // The location is solid if the map cell is nonzero OR if we're colliding with
    // any (solid) bounding boxes
    uint8_t tile = raycast.worldMap.getCell(x.getInteger(), y.getInteger());

    return (tile != 0) || raycast.sprites.firstColliding(x, y, RBSTATESOLID) != NULL;
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

    if (arduboy.pressed(B_BUTTON))
    {
        //We're mean and always drain sprint meter if holding B
        sprintmeter -= SPRINTDRAIN;

        if(sprintmeter < 0)
            sprintmeter = 0;

        //You're allowed to run if you're already running and there's any sprint left,
        //or if you 'just started' running and there's minimum sprint meter
        if(holding_b && sprintmeter > 0 || sprintmeter > SPRINTMIN)
        {
            rotation *= RUNMULTIPLIER;
            movement *= RUNMULTIPLIER;
        }

        //End with us knowing if they're holding B
        holding_b = true;
    }
    else
    {
        sprintmeter = min(sprintmeter + SPRINTRECOVER, SPRINTMAX);
        holding_b = false;
    }

    raycast.player.tryMovement(movement, rotation, &isSolid);

    bool reload = false;

    int8_t ofs_x = ((flot)raycast.player.posX - CAGEX).getInteger();
    int8_t ofs_y = ((flot)raycast.player.posY - CAGEY).getInteger();

    if(ofs_x) {
        world_x += ofs_x;
        raycast.player.posX -= ofs_x;
        reload = true;
    }
    if(ofs_y) {
        world_y += ofs_y;
        raycast.player.posY -= ofs_y;
        reload = true;
    }

    if(reload)
    {
        shift_sprites(ofs_x, ofs_y);
        load_region();
    }
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

//Shift ALL sprites by the given amount in x and y (whole numbers only please!). Also
//erases sprites that go outside the usable area
void shift_sprites(int8_t x, int8_t y)
{
    for(uint8_t i = 0; i < NUMSPRITES; i++)
    {
        RcSprite<NUMINTERNALBYTES> * sp = raycast.sprites[i];
        if(sp->isActive())
        {
            sp->x += x;
            sp->y += y;

            if(sp->x < SPRITEBEGIN_X || sp->y < SPRITEBEGIN_Y || sp->x >= SPRITEEND_X + 1 || sp->y >= SPRITEEND_Y + 1)    
                raycast.sprites.deleteSprite(sp);
        }
    }
    //for(uint8_t i = 0; i < SPRITEGC_PERFRAME; i++)
    //{
    //    RcSprite<NUMINTERNALBYTES> * sp = raycast.sprites[spritegc_i];
    //    if(sp->isActive())
    //    {
    //        if(sp->x < SPRITEBEGIN_X || sp->y < SPRITEBEGIN_Y || sp->x >= SPRITEEND_X + 1 || sp->y >= SPRITEEND_Y + 1)    
    //            raycast.sprites.deleteSprite(sp);
    //    }
    //    spritegc_i = (spritegc_i + 1) % NUMSPRITES;
    //}
}

// Reload the map for the local region. We'll see if this is too slow...
void load_region()
{
    int16_t left = world_x - CAGEX;
    int16_t top = world_y - CAGEY;
    int16_t right = CAGEX + world_x;
    int16_t bottom = CAGEY + world_y;

    uint8_t world_begin_x, world_begin_y;
    uint8_t map_begin_x, map_begin_y;
    uint8_t map_end_x = LOADWIDTH - 1 - (right >= staticmap_width ? 1 + right - staticmap_width : 0);
    uint8_t map_end_y = LOADHEIGHT - 1 - (bottom >= staticmap_height ? 1 + bottom - staticmap_height : 0);

    if(left < 0) { 
        world_begin_x = 0; 
        map_begin_x = -left; 
    }
    else { 
        world_begin_x = left; 
        map_begin_x = 0; 
    }
    if(top < 0) { 
        world_begin_y = 0; 
        map_begin_y = -top; 
    }
    else { 
        world_begin_y = top; 
        map_begin_y = 0; 
    }

    uint8_t writelen = 1 + map_end_x - map_begin_x;
    for(uint8_t y = map_begin_y; y <= map_end_y; y++) {
        FX::readDataBytes(staticmap_fx + (world_begin_y++ * staticmap_width + world_begin_x), raycast.worldMap.map + (y * RCMAXMAPDIMENSION + map_begin_x), writelen);
    }
}

void setup()
{
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.flashlight();
    arduboy.initRandomSeed();
    arduboy.setFrameRate(FRAMERATE);
    FX::begin(FX_DATA_PAGE);    // initialise FX chip

    //raycast.render.setLightIntensity(1.5);

    //raycast.render.spritescaling[0] = d
    raycast.player.posX = CAGEX + 0.5;
    raycast.player.posY = CAGEY + 0.5;

    load_region();

    drawMenu(false);
}


void loop()
{
    if(!arduboy.nextFrame()) return;

    // Process player movement + interaction
    movement();

    // Draw the correct background for the area. 
    //drawMenu(false);
    raycast.render.drawRaycastBackground(&arduboy, raycastBg);

    raycast.runIteration(&arduboy);

    raycast.worldMap.drawMap(&arduboy, 105, 0);
    //tinyfont.setCursor(0,0);
    //tinyfont.print(sprintmeter);

    FX::display(false);

    //Figure out sprite situation
    //sprite_gc();
}
