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
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;

// Since we're using this number so many times in template types, might 
// as well make it a constant.
constexpr uint8_t NUMINTERNALBYTES = 1;
constexpr uint8_t NUMSPRITES = 16;

// Some stuff for external map loading
constexpr uint8_t CAGEX = 7;
constexpr uint8_t CAGEY = 7;
constexpr uint8_t LOADWIDTH = 15;
constexpr uint8_t LOADHEIGHT = 15;

uint8_t world_x = CAGEX + 1;
uint8_t world_y = CAGEY;

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

    raycast.player.tryMovement(movement, rotation, &isSolid);

    bool reload = false;

    flot ofs_x = (flot)raycast.player.posX - CAGEX;
    flot ofs_y = (flot)raycast.player.posY - CAGEY;

    if(abs(ofs_x) >= 1) {
        int8_t movement = ofs_x.getInteger();
        world_x += movement;
        raycast.player.posX -= movement;
        reload = true;
    }
    if(abs(ofs_y) >= 1) {
        int8_t movement = ofs_y.getInteger();
        world_y += movement;
        raycast.player.posY -= movement;
        reload = true;
    }

    if(reload)
        load_region();
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

    raycast.render.setLightIntensity(1.5);

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

    //raycast.worldMap.drawMap(&arduboy, 105, 0);

    FX::display(false);
}
