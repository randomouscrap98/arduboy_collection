#include <Tinyfont.h>
#include <FixedPoints.h>
#include <Arduboy2.h>
//#include <ArduboyTones.h>

#include <math.h>

// Graphics
#include "menu.h"
#include "light2.h"

// Libs (sort of; mostly just code organization)
#include "mazedef.h"
#include "mazegen.h"

Arduboy2Base arduboy;
//ArduboyTones sound(arduboy.audio.enabled);
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

// Define a fake FOV. Slightly more computation but not much honestly. 
// It just doesn't do much. 1 = "90", more than 1 = more than 90
// #define FAKEFOV 0.8

// Define a new wallheight. This usually doesn't look great, and it
// wastes cycles, so I made it a define
// #define WALLHEIGHT 1.0

// I modify these for testing, just nice to have it abstracted
typedef SFixed<7,8> flot;
typedef UFixed<8,8> uflot;

// Gameplay constants
constexpr uint8_t FRAMERATE = 45; //Untextured can run at near 60; lots of headroom at 45
constexpr float MOVESPEED = 4.0f / FRAMERATE;
constexpr float ROTSPEED = 4.0f / FRAMERATE;
constexpr uflot LIGHTINTENSITY = 1.5;

//Visual constants (Probably shouldn't change these)
constexpr uint8_t LIGHTSTART = 34;
constexpr uint8_t BAYERGRADIENTS = 16;

//These are calculated constants based off your light intensity. The view 
//distance is an optimization; increase light intensity to increase view distance.
//We calculate "darkness" to avoid 100 divisions per frame (huuuge savings)
const uflot VIEWDISTANCE = sqrt(BAYERGRADIENTS * (float)LIGHTINTENSITY);
const uflot DARKNESS = 1 / LIGHTINTENSITY;

// Funny hack constants. We're working with very small ranges, so 
// the values have to be picked carefully. The following must remain true:
// 1 / NEARZEROFIXED < MAXFIXED. It may even need to be < MAXFIXED / 2
constexpr uflot MAXFIXED = 255;
constexpr uflot NEARZEROFIXED = 1.0f / 128; // Prefer accuracy (fixed decimal exact)

//Screen calc constants (no need to do it at runtime)
constexpr uint8_t MIDSCREEN = HEIGHT / 2;
constexpr uint8_t VIEWWIDTH = 100;
constexpr flot NORMWIDTH = 2.0f / VIEWWIDTH;

uint8_t worldMap[MAXMAPHEIGHT * MAXMAPWIDTH / TILESPERBYTE];
//[MAXMAPWIDTH]=
//{
//  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
//  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
//  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
//  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
//  {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
//  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
//  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
//  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
//  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
//  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
//  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
//  {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
//  {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
//  {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
//  {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
//  {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
//  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
//  {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
//  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
//  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
//  {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
//  {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
//  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
//  {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
//};

// Bayer gradients, not including the 0 fill (useless?).
// Takes up 64 precious bytes of RAM
constexpr uint8_t b_shading[] = {
    0xFF, 0xFF, 0xFF, 0xFF, // Beyer 16
    0xEE, 0xFF, 0xFF, 0xFF, // 0
    0xEE, 0xFF, 0xBB, 0xFF,
    0xEE, 0xFF, 0xAA, 0xFF, // 2 
    0xAA, 0xFF, 0xAA, 0xFF, 
    0xAA, 0xDD, 0xAA, 0xFF, // 4
    0xAA, 0xDD, 0xAA, 0x77,
    0xAA, 0xDD, 0xAA, 0x55, // 6
    0xAA, 0x55, 0xAA, 0x55,
    0xAA, 0x44, 0xAA, 0x55, // 8
    0xAA, 0x44, 0xAA, 0x11, 
    0xAA, 0x44, 0xAA, 0x00, // 10
    0xAA, 0x00, 0xAA, 0x00, 
    0x44, 0x00, 0xAA, 0x00, // 12
    0x44, 0x00, 0x22, 0x00,
    0x44, 0x00, 0x00, 0x00, // 14
};

//Menu related stuff
uint8_t menuIndex = 0;
uint8_t mazeSize = 0;
uint8_t mazeType = 0;

float posX, posY; // = 1.6, posY = 1.6;   //x and y start position
//float posX = 22, posY = 11.6;   //x and y start position
float dirX, dirY; //dirX = -1, dirY = 0;      //initial direction vector

// The full function for raycasting. 
void raycast()
{
    //Waste 10 bytes of stack to save numerous cycles on render (and on programmer. leaving posX + posY floats so...)
    uint8_t pmapX = int(posX);
    uint8_t pmapY = int(posY);
    uflot pmapofsX = posX - pmapX;
    uflot pmapofsY = posY - pmapY;
    flot dx = dirX; //NO floating points inside critical loop!!
    flot dy = dirY;

    for (uint8_t x = 0; x < VIEWWIDTH; x++)
    {
        flot cameraX = (flot)x * NORMWIDTH - 1; // x-coordinate in camera space

        // The camera plane is a simple -90 degree rotation on the player direction (as required for this algorithm).
        // As such, it's simply (dirY, -dirX) * FAKEFOV. The camera plane does NOT need to be tracked separately
        #ifdef FAKEFOV
        flot rayDirX = dx + dy * FAKEFOV * cameraX;
        flot rayDirY = dy - dx * FAKEFOV * cameraX;
        #else
        flot rayDirX = dx + dy * cameraX;
        flot rayDirY = dy - dx * cameraX;
        #endif

        // length of ray from one x or y-side to next x or y-side. But we prefill it with
        // some initial data which has to be massaged later.
        uflot deltaDistX = (uflot)abs(rayDirX); //Temp value; may not be used
        uflot deltaDistY = (uflot)abs(rayDirY); //same

        // length of ray from current position to next x or y-side
        uflot sideDistX = MAXFIXED;
        uflot sideDistY = MAXFIXED;

        // what direction to step in x or y-direction (either +1 or -1)
        int8_t stepX = 0;
        int8_t stepY = 0;

        // With this DDA stepping algorithm, have to be careful about making too-large values
        // with our tiny fixed point numbers. Make some arbitrarily small cutoff point for
        // even trying to deal with steps in that direction. As long as the map size is 
        // never larger than 1 / NEARZEROFIXED on any side, it will be fine (that meeans
        // map has to be < 100 on a side with this)
        if(deltaDistX > NEARZEROFIXED) {
            deltaDistX = 1 / deltaDistX;
            if (rayDirX < 0) {
                stepX = -1;
                sideDistX = pmapofsX * deltaDistX;
            }
            else {
                stepX = 1;
                sideDistX = (1 - pmapofsX) * deltaDistX;
            }
        }
        if(deltaDistY > NEARZEROFIXED) {
            deltaDistY = 1 / deltaDistY;
            if (rayDirY < 0) {
                stepY = -1;
                sideDistY = pmapofsY * deltaDistY;
            }
            else {
                stepY = 1;
                sideDistY = (1 - pmapofsY) * deltaDistY;
            }
        }

        uint8_t side;           // was a NS or a EW wall hit?
        uint8_t mapX = pmapX;   // which box of the map the ray collision is in
        uint8_t mapY = pmapY;
        uflot perpWallDist = 0;

        // perform DDA
        while (perpWallDist < VIEWDISTANCE)
        {
            // jump to next map square, either in x-direction, or in y-direction
            if (sideDistX < sideDistY) {
                perpWallDist = sideDistX; // Remember that sideDist is actual distance and not distance only in 1 direction
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0; //0 = xside hit
            }
            else {
                perpWallDist = sideDistY;
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1; //1 = yside hit
            }
            // Check if ray has hit a wall
            if (getMazeCell(worldMap, mapX, mapY)) //[mapX][mapY])
                break;
        }

        if(perpWallDist >= VIEWDISTANCE) continue;

        // Calculate half height of line to draw on screen. We already know the distance to the wall.
        // We can truncate the total height if too close to the wall right here and now and avoid future checks.
        #ifdef WALLHEIGHT
        uint8_t lineHeight = (perpWallDist < WALLHEIGHT ? HEIGHT : (int)(HEIGHT / perpWallDist * WALLHEIGHT)) >> 1;
        #else
        uint8_t lineHeight = (perpWallDist <= 1 ? HEIGHT : (int)(HEIGHT / perpWallDist)) >> 1;
        #endif

        //NOTE: unless view distance is set to > 32, lineHeight will never be 0, so no need to check.
        //ending should be exclusive
        draw_wall_line(x, MIDSCREEN - lineHeight, MIDSCREEN + lineHeight, perpWallDist, side);
    }
}

//Draw a single raycast wall line. Will only draw specifically the wall line and will clip out all the rest
//(so you can predraw a ceiling and floor before calling raycast)
inline void draw_wall_line(uint8_t x, uint8_t yStart, uint8_t yEnd, uflot distance, uint8_t side) 
{
    //NOTE: multiplication is WAY FASTER than division
    uint8_t dither = (uint8_t)(roundFixed(distance * DARKNESS * distance));

    if(dither >= BAYERGRADIENTS)
        return;

    uint8_t shade = (side & x) ? 0 : b_shading[(dither << 2) + (x & 3)];
    uint8_t start = yStart >> 3;
    uint8_t end = (yEnd - 1) >> 3; //This end needs to be inclusive

    for(uint8_t b = start; b <= end; ++b)
    {
        uint8_t m = 0xFF;
        if(b == start)
            m &= (0xFF << (yStart & 7));
        if(b == end && yEnd & 7) //Short circuit I sincerely hope!!
            m &= (0xFF << (yEnd & 7)) >> 8;
        arduboy.sBuffer[b * WIDTH + x] = (arduboy.sBuffer[b * WIDTH + x] & ~m) | (shade & m);
    }
}

void raycastFoundation()
{
    //Arduboy2 fillrect is absurdly slow; I have the luxury of doing this instead
    for(uint8_t i = 0; i <= LIGHTSTART >> 3; ++i)
        memset(arduboy.sBuffer + i * WIDTH, 0, VIEWWIDTH);
    Sprites::drawOverwrite(0, LIGHTSTART, light, 0);
}

void movement()
{
    // move forward if no wall in front of you
    if (arduboy.pressed(A_BUTTON))
    {
        if (!getMazeCell(worldMap, (int)(posX + dirX * MOVESPEED), (int)posY)) //worldMap[int(posX + dirX * MOVESPEED)][int(posY)] == false)
            posX += dirX * MOVESPEED;
        if (!getMazeCell(worldMap, (int)posX, (int)(posY + dirY * MOVESPEED))) //(worldMap[int(posX)][int(posY + dirY * MOVESPEED)] == false)
            posY += dirY * MOVESPEED;
    }
    // rotate to the right
    if (arduboy.pressed(RIGHT_BUTTON))
    {
        // both camera direction and camera plane must be rotated
        float oldDirX = dirX;
        dirX = dirX * cos(-ROTSPEED) - dirY * sin(-ROTSPEED);
        dirY = oldDirX * sin(-ROTSPEED) + dirY * cos(-ROTSPEED);
    }
    // rotate to the left
    if (arduboy.pressed(LEFT_BUTTON))
    {
        // both camera direction and camera plane must be rotated
        float oldDirX = dirX;
        dirX = dirX * cos(ROTSPEED) - dirY * sin(ROTSPEED);
        dirY = oldDirX * sin(ROTSPEED) + dirY * cos(ROTSPEED);
    }
}

void doMenu()
{
    constexpr uint8_t MENUITEMS = 3;
    int8_t menuMod = 0;
    int8_t selectMod = 0;

    if(arduboy.justPressed(UP_BUTTON))
        menuMod = -1;
    if(arduboy.justPressed(DOWN_BUTTON))
        menuMod = 1;

    menumod(menuIndex, menuMod, MENUITEMS);

    if(arduboy.justPressed(B_BUTTON))
    {
        selectMod = 1;
        switch (menuIndex)
        {
            case 0:
                menumod(mazeSize, selectMod, MAZESIZECOUNT);
                break;
            case 1:
                menumod(mazeType, selectMod, MAZETYPECOUNT);
                break;
            case 2:
                generateMaze();
                break;
        }
    }

    if(menuMod || selectMod)
        drawMenu();
}

void drawMenu()
{
    constexpr uint8_t MENUX = 105;
    constexpr uint8_t MENUY = 24;
    constexpr uint8_t MENUSPACING = 6;

    Sprites::drawOverwrite(VIEWWIDTH, 0, menu, 0);
    tinyfont.setCursor(109, 4);
    tinyfont.print(F("3D"));
    tinyfont.setCursor(105, 9);
    tinyfont.print(F("Maze"));

    MazeSize mzs = getMazeSize(mazeSize);
    tinyfont.setCursor(MENUX + 4, MENUY);
    tinyfont.print(mzs.name);

    MazeType mzt = MAZETYPES[mazeType];
    tinyfont.setCursor(MENUX + 4, MENUY + MENUSPACING);
    tinyfont.print(mzt.name);

    tinyfont.setCursor(MENUX + 4, MENUY + MENUSPACING * 2);
    tinyfont.print(F("NEW"));

    tinyfont.setCursor(MENUX, MENUY + menuIndex * MENUSPACING);
    tinyfont.print("o");
}

void generateMaze()
{
    arduboy.clear();
    tinyfont.setCursor(40, 28);
    tinyfont.print(F("Generating maze"));
    arduboy.display();

    MazeSize mzs = getMazeSize(mazeSize);
    MazeType mzt = MAZETYPES[mazeType];

    mzt.func(worldMap, mzs.width, mzs.height);
    posX = 1.6;
    posY = 1.6;
    dirX = 1;
    dirY = 0;
}

//Using floats for now, will use others later
void setup()
{
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.flashlight();
    arduboy.initRandomSeed();
    arduboy.clear();
    arduboy.setFrameRate(FRAMERATE);
    generateMaze();
    drawMenu();
}

void loop()
{
    if (!arduboy.nextFrame()) return;

    arduboy.pollButtons();
    raycastFoundation();
    raycast();
    movement();
    doMenu();
    arduboy.display();
}
