#include <FixedPoints.h>

#include <Arduboy2.h>
#include <ArduboyTones.h>

#include <math.h>
#include <float.h>

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);

// Define a fake FOV. Slightly more computation but not much honestly. 
// It just doesn't do much. 1 = "90", more than 1 = more than 90
// #define FAKEFOV 1.0

// Define a new wallheight. This usually doesn't look great, and it
// wastes cycles, so I made it a define
// #define WALLHEIGHT 1.0

// I modify these for testing, just nice to have it abstracted
typedef SFixed<7,8> flot;
typedef UFixed<8,8> uflot;

// Funny hack constants. We're working with very small ranges, so 
// the values have to be picked carefully. The following must remain true:
// 1 / NEARZEROFIXED < MAXFIXED. It may even need to be < MAXFIXED / 2
constexpr uflot MAXFIXED = 255;
constexpr uflot NEARZEROFIXED = 0.01;

// Screen calc constants (no need to do it at runtime)
constexpr uint8_t MIDSCREEN = HEIGHT / 2;
constexpr uint8_t VIEWWIDTH = 100;
constexpr float NORMWIDTH = 2.0f / VIEWWIDTH;

constexpr uint8_t BAYERGRADIENTS = 16;

// Gameplay constants
constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 5.0f / FRAMERATE;
constexpr float ROTSPEED = 4.0f / FRAMERATE;
constexpr uint8_t VIEWDISTANCE = 16;
constexpr uflot LIGHTINTENSITY = 0.08;

constexpr uint8_t MAPWIDTH = 24;
constexpr uint8_t MAPHEIGHT = 24;

uint8_t worldMap[MAPHEIGHT][MAPWIDTH]=
{
  //{0b11111111, 0b11111111, 0b11111111},
  //{0b00000001, 0b00000000, 0b10000000},
  //{0b00000001, 0b00000000, 0b10000000},
  //{0b00000001, 0b00000000, 0b10000000},
  //{0b11000001, 0b10000111, 0b10001010},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,2,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,3,0,0,0,3,0,0,0,1},
  {1,0,0,0,0,0,2,0,0,0,2,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,2,2,0,2,2,0,0,0,0,3,0,3,0,3,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,5,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,0,0,0,0,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,4,4,4,4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
  {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1}
};

// Sprite-based gradients. May change to direct buffer writing for 
// true gradients
//constexpr uint8_t GRADIENTS = 4;
//constexpr uint8_t sp_shading[] PROGMEM = { 
//    1, 64,
//    255,255,255,255,255,255,255,255,
//    255,255,255,255,255,255,255,255,
//
//    238,238,238,238,238,238,238,238,
//    187,187,187,187,187,187,187,187,
//
//    170,170,170,170,170,170,170,170,
//    85,85,85,85,85,85,85,85,
//
//    136,136,136,136,136,136,136,136,
//    34,34,34,34,34,34,34,34,
//};

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

// Top and bottom masking for bytes to fill screen for dithering
constexpr uint8_t b_masks[] = {
    0xFF, 0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80,
    0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 0x07, 0x03, 0x01
};

float posX = 22, posY = 12;     //x and y start position
float dirX = 0, dirY = -1;      //initial direction vector

// The full function for rendering walls. It's inlined because I only ever expect to call it from
// one location. If that changes, I'll just remove inline, it'll be VERY OBVIOUS in the output size
inline void render_walls()
{
    //Waste 6 bytes of memory to save numerous cycles on render (and on programmer. leaving posX + posY floats so...)
    uint8_t pmapX = int(posX);
    uint8_t pmapY = int(posY);
    uflot pmapofsX = posX - pmapX;
    uflot pmapofsY = posY - pmapY;

    for (uint8_t x = 0; x < VIEWWIDTH; x++)
    {
        // calculate ray position and direction. This is the only place we use floats,
        // so maybe 7 * 64 float operations per frame. Maybe that's OK? 
        flot cameraX = x * NORMWIDTH - 1; // x-coordinate in camera space

        // The camera plane is a simple -90 degree rotation on the player direction (as required for this algorithm).
        // As such, it's simply (dirY, -dirX) * FAKEFOV. The camera plane does NOT need to be tracked separately
        #ifdef FAKEFOV
        flot rayDirX = dirX + dirY * FAKEFOV * cameraX;
        flot rayDirY = dirY - dirX * FAKEFOV * cameraX;
        #else
        flot rayDirX = dirX + dirY * cameraX;
        flot rayDirY = dirY - dirX * cameraX;
        #endif

        // which box of the map the ray collision is in
        uint8_t mapX = pmapX;
        uint8_t mapY = pmapY;

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

        uflot perpWallDist = 0;
        uint8_t side;    // was a NS or a EW wall hit?

        // perform DDA
        while (perpWallDist < VIEWDISTANCE)
        {
            // jump to next map square, either in x-direction, or in y-direction
            if (sideDistX < sideDistY) {
                perpWallDist = sideDistX; // Remember that sideDist is actual distance and not distance only in 1 direction
                sideDistX += deltaDistX;
                mapX += stepX;
                side = 0;
            }
            else {
                perpWallDist = sideDistY;
                sideDistY += deltaDistY;
                mapY += stepY;
                side = 1;
            }
            // Check if ray has hit a wall
            if (worldMap[mapX][mapY])
                break;
        }

        if(perpWallDist >= VIEWDISTANCE || side & x) continue;

        // Calculate half height of line to draw on screen. We already know the distance to the wall.
        // We can truncate the total height if too close to the wall right here and now and avoid future checks.
        #ifdef WALLHEIGHT
        uint8_t lineHeight = (perpWallDist < WALLHEIGHT ? HEIGHT : (int)(HEIGHT / perpWallDist * WALLHEIGHT)) >> 1;
        #else
        uint8_t lineHeight = (perpWallDist <= 1 ? HEIGHT : (int)(HEIGHT / perpWallDist)) >> 1;
        #endif

        //NOTE: unless view distance is set to > 32, lineHeight will never be 0, so no need to check
        draw_wall_line(x, MIDSCREEN - lineHeight, MIDSCREEN + lineHeight - 1, perpWallDist);
    }
}

inline void draw_wall_line(uint8_t x, uint8_t yStart, uint8_t yEnd, uflot distance)
{
    uint8_t dither = (distance / VIEWDISTANCE * distance / LIGHTINTENSITY).getInteger();

    if(dither >= 16)
        return;

    uint8_t shade = b_shading[(dither << 2) + (x & 3)];
    for(uint8_t b = (yStart >> 3); b <= (yEnd >> 3); ++b)
    {
        uint16_t index = b * WIDTH + x;
        arduboy.sBuffer[index] = shade;

        if(index >= 1024) {
            sound.tone(440, 32);
        }
    }

    if(yStart > 0)
        arduboy.drawFastVLine(x, 0, yStart, BLACK);
    if(yEnd < HEIGHT - 1)
        arduboy.drawFastVLine(x, yEnd + 1, HEIGHT - 1 - yEnd, BLACK);
}

// inline void draw_wall_line(uint8_t x, uint8_t yStart, uint8_t yEnd, uflot distance)
// {
//     //Choose wall color based sort of on distance + wall side
//     uint8_t color_offset = (distance / LIGHTINTENSITY).getInteger();
// 
//     if(color_offset >= GRADIENTS)
//         color_offset = GRADIENTS - 1;
// 
//     Sprites::drawOverwrite(x, yStart, sp_shading, (color_offset << 1) + (x & 1));
//     if (yEnd < HEIGHT - 1)
//         arduboy.drawFastVLine(x, yEnd + 1, HEIGHT - 1, BLACK);
// }

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
    arduboy.clear();
    arduboy.drawRect(VIEWWIDTH + 1, 0, WIDTH - VIEWWIDTH - 2, HEIGHT, WHITE);
    //arduboy.setCursor(108, 1);
    //sprintf(buff, "%d", light);
    //arduboy.print("DE");
    render_walls();
    movement();
    arduboy.display();
}
