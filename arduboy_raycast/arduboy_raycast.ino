#include <FixedPoints.h>
#include <Arduboy2.h>
#include <ArduboyTones.h>

#include <math.h>

#include "menu.h"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

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
constexpr uint8_t FRAMERATE = 30; //Untextured can run at near 60; definitely 45
constexpr float MOVESPEED = 4.0f / FRAMERATE;
constexpr float ROTSPEED = 4.0f / FRAMERATE;
constexpr uflot LIGHTINTENSITY = 1.5;

// Funny hack constants. We're working with very small ranges, so 
// the values have to be picked carefully. The following must remain true:
// 1 / NEARZEROFIXED < MAXFIXED. It may even need to be < MAXFIXED / 2
constexpr uflot MAXFIXED = 255;
constexpr uflot NEARZEROFIXED = 1.0f / 128; // Prefer accuracy (fixed decimal exact)

// Screen calc constants (no need to do it at runtime)
constexpr uint8_t MIDSCREEN = HEIGHT / 2;
constexpr uint8_t VIEWWIDTH = 100;
constexpr flot NORMWIDTH = 2.0f / VIEWWIDTH;

constexpr uint8_t BAYERGRADIENTS = 16;

//This is a calculated constant based off your light intensity. The view 
//distance is an optimization; increase light intensity to increase view distance.
const uflot VIEWDISTANCE = sqrt(BAYERGRADIENTS * (float)LIGHTINTENSITY);
const uflot DARKNESS = 1 / LIGHTINTENSITY;

constexpr uint8_t MAPWIDTH = 24;
constexpr uint8_t MAPHEIGHT = 24;

uint8_t worldMap[MAPHEIGHT][MAPWIDTH]=
{
  //{0b11111111, 0b11111111, 0b11111111},
  //{0b00000001, 0b00000000, 0b10000000},
  //{0b00000001, 0b00000000, 0b10000000},
  //{0b00000001, 0b00000000, 0b10000000},
  //{0b11000001, 0b10000111, 0b10001010},
  {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,7,7,7,7,7,7,7,7},
  {4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7},
  {4,0,3,0,0,0,0,0,0,0,0,0,0,0,0,0,7,0,0,0,0,0,0,7},
  {4,0,4,0,0,0,0,5,5,5,5,5,5,5,5,5,7,7,0,7,7,7,7,7},
  {4,0,5,0,0,0,0,5,0,5,0,5,0,5,0,5,7,0,0,0,7,7,7,1},
  {4,0,6,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,7,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,7,7,7,1},
  {4,0,8,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,0,0,0,8},
  {4,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,7,0,0,0,7,7,7,1},
  {4,0,0,0,0,0,0,5,5,5,5,0,5,5,5,5,7,7,7,7,7,7,7,1},
  {6,6,6,6,6,6,6,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {8,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,4},
  {6,6,6,6,6,6,0,6,6,6,6,0,6,6,6,6,6,6,6,6,6,6,6,6},
  {4,4,4,4,4,4,0,4,4,4,6,0,6,2,2,2,2,2,2,2,3,3,3,3},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,0,0,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,0,0,0,0,5,0,0,0,0,0,0,2},
  {4,0,0,5,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,2,0,2,2},
  {4,0,6,0,6,0,0,0,0,4,6,0,6,2,0,0,5,0,0,2,0,0,0,2},
  {4,0,0,0,0,0,0,0,0,4,6,0,6,2,0,0,0,0,0,2,0,0,0,2},
  {4,4,4,4,4,4,4,4,4,4,1,1,1,2,2,2,2,2,2,3,3,3,3,3}
};

// Bayer gradients, not including the 0 fill (useless?)
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

float posX = 22, posY = 11.6;   //x and y start position
float dirX = -1, dirY = 0;      //initial direction vector

// The full function for raycasting. It's inlined because I only ever expect to call it from
// one location. If that changes, I'll just remove inline, it'll be VERY OBVIOUS in the output size
inline void raycast()
{
    //Waste 10 bytes of stack to save numerous cycles on render (and on programmer. leaving posX + posY floats so...)
    uint8_t pmapX = int(posX);
    uint8_t pmapY = int(posY);
    uflot pmapofsX = posX - pmapX;
    uflot pmapofsY = posY - pmapY;
    flot dx = dirX; //NO floating points inside critical loop!!
    flot dy = dirY;

    //uflot dx2dy = 4 * ((uflot)abs(dx) + 1) / ((uflot)abs(dy) + 1);
    //uflot dy2dx = 4 * ((uflot)abs(dy) + 1) / ((uflot)abs(dx) + 1);

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
            if (worldMap[mapX][mapY])
                break;
        }

        //if(perpWallDist >= VIEWDISTANCE || side & x) continue;
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
        draw_wall_line(x, MIDSCREEN - lineHeight, MIDSCREEN + lineHeight, perpWallDist, side); // ? 4 : 0); //dx2dy : dy2dx);
    }
}

inline void draw_wall_line(uint8_t x, uint8_t yStart, uint8_t yEnd, uflot distance, uint8_t side) //uflot dim)
{
    //NOTE: multiplication is WAY FASTER than division
    uint8_t dither = (uint8_t)(roundFixed(distance * DARKNESS * distance)); // + dim);

    if(dither >= BAYERGRADIENTS)
        return;

    //uint8_t shade = b_shading[(dither << 2) + (x & 3)];
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

inline void movement()
{
    // move forward if no wall in front of you
    if (arduboy.pressed(UP_BUTTON))
    {
        if (worldMap[int(posX + dirX * MOVESPEED)][int(posY)] == false)
            posX += dirX * MOVESPEED;
        if (worldMap[int(posX)][int(posY + dirY * MOVESPEED)] == false)
            posY += dirY * MOVESPEED;
    }
    // move backwards if no wall behind you
    if (arduboy.pressed(DOWN_BUTTON))
    {
        if (worldMap[int(posX - dirX * MOVESPEED)][int(posY)] == false)
            posX -= dirX * MOVESPEED;
        if (worldMap[int(posX)][int(posY - dirY * MOVESPEED)] == false)
            posY -= dirY * MOVESPEED;
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
    //if (arduboy.justPressed(B_BUTTON))
    //{
    //    light = (light + 1) & 7;
    //}
}

//Using floats for now, will use others later
void setup()
{
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.initRandomSeed();
    arduboy.clear();
    arduboy.setFrameRate(FRAMERATE);
}


//char buff[20];
void loop()
{
    if (!arduboy.nextFrame()) return;

    arduboy.pollButtons();
    //arduboy.clear();
    constexpr uint16_t cutoff = 1024; //640;
    memset(arduboy.sBuffer, 0x00, cutoff);
    memset(arduboy.sBuffer + cutoff, 0xAA, 1024 - cutoff);
    Sprites::drawOverwrite(VIEWWIDTH, 0, menu, 0);
    arduboy.setCursor(106, 4);
    arduboy.print("END");
    raycast();
    movement();
    arduboy.display();
}
