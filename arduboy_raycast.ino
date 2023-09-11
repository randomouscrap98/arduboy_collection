#include <FixedPoints.h>

#include <Arduboy2.h>
#include <math.h>
#include <float.h>

Arduboy2 arduboy;


typedef SFixed<7,8> flot;
typedef UFixed<8,8> uflot;
constexpr uflot MAXFIXED = 255;
constexpr uflot NEARZEROFIXED = 0.01;


constexpr uint8_t MIDSCREEN = HEIGHT / 2;
constexpr uflot NORMWIDTH = 2.0f / WIDTH;

constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 5.0f / FRAMERATE;
constexpr float ROTSPEED = 4.0f / FRAMERATE;

constexpr uint8_t VIEWDISTANCE = 16;
constexpr uflot LIGHTINTENSITY = 2;

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

constexpr uint8_t GRADIENTS = 4;
constexpr uint8_t sp_shading[] PROGMEM = { 
    1, 64,
    255,255,255,255,255,255,255,255,
    255,255,255,255,255,255,255,255,

    238,238,238,238,238,238,238,238,
    187,187,187,187,187,187,187,187,

    170,170,170,170,170,170,170,170,
    85,85,85,85,85,85,85,85,

    136,136,136,136,136,136,136,136,
    34,34,34,34,34,34,34,34,
};

float posX = 22, posY = 12;  //x and y start position
float dirX = -1, dirY = 0; //initial direction vector
//TODO: figure out what part of the algorithm is broken such that any values 
//other than dirX = -1 and dirY = 0 break everything.
//float dirX = 0, dirY = 1; //initial direction vector
float planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

inline void render()
{
    //Waste 6 bytes of memory to save numerous cycles on render (leaving posX + posY floats so...)
    uint8_t pmapX = int(posX);
    uint8_t pmapY = int(posY);
    uflot pmapofsX = posX - pmapX;
    uflot pmapofsY = posY - pmapY;

    for (uint8_t x = 0; x < WIDTH; x++)
    {
        // calculate ray position and direction. This is the only place we use floats,
        // so maybe 7 * 64 float operations per frame. Maybe that's OK? 
        flot cameraX = x * 2.0f / WIDTH - 1; // x-coordinate in camera space
        flot rayDirX = dirX + planeX * cameraX;
        flot rayDirY = dirY + planeY * cameraX;

        // which box of the map the ray collision is in
        uint8_t mapX = pmapX;
        uint8_t mapY = pmapY;

        // length of ray from one x or y-side to next x or y-side
        // these are derived as:
        // deltaDistX = sqrt(1 + (rayDirY * rayDirY) / (rayDirX * rayDirX))
        // deltaDistY = sqrt(1 + (rayDirX * rayDirX) / (rayDirY * rayDirY))
        // which can be simplified to abs(|rayDir| / rayDirX) and abs(|rayDir| / rayDirY)
        // where |rayDir| is the length of the vector (rayDirX, rayDirY). Its length,
        // unlike (dirX, dirY) is not 1, however this does not matter, only the
        // ratio between deltaDistX and deltaDistY matters, due to the way the DDA
        // stepping further below works. So the values can be computed as below.
        //  Division through zero is prevented, even though technically that's not
        //  needed in C++ with IEEE 754 floating point values.
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

        //Choose wall color based sort of on distance + wall side
        uint8_t color_offset = (perpWallDist / LIGHTINTENSITY).getInteger();

        if(color_offset >= GRADIENTS)
            color_offset = GRADIENTS - 1;

        // Calculate half height of line to draw on screen. We already know the distance to the wall
        uint8_t lineHeight = (perpWallDist < 1 ? HEIGHT : (int)(HEIGHT / perpWallDist)) >> 1;

        // calculate lowest and highest pixel to fill in current stripe
        uint8_t drawStart = MIDSCREEN - lineHeight;
        uint8_t drawEnd = MIDSCREEN + lineHeight;

        Sprites::drawOverwrite(x, drawStart, sp_shading, (color_offset << 1) + (x & 1));
        if (drawEnd < HEIGHT - 1)
            arduboy.drawFastVLine(x, drawEnd + 1, HEIGHT - 1, BLACK);
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
        float oldPlaneX = planeX;
        planeX = planeX * cos(-ROTSPEED) - planeY * sin(-ROTSPEED);
        planeY = oldPlaneX * sin(-ROTSPEED) + planeY * cos(-ROTSPEED);
    }
    // rotate to the left
    if (arduboy.pressed(LEFT_BUTTON))
    {
        // both camera direction and camera plane must be rotated
        float oldDirX = dirX;
        dirX = dirX * cos(ROTSPEED) - dirY * sin(ROTSPEED);
        dirY = oldDirX * sin(ROTSPEED) + dirY * cos(ROTSPEED);
        float oldPlaneX = planeX;
        planeX = planeX * cos(ROTSPEED) - planeY * sin(ROTSPEED);
        planeY = oldPlaneX * sin(ROTSPEED) + planeY * cos(ROTSPEED);
    }
}

//Using floats for now, will use others later
void setup()
{
    // arduboy.begin();
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.initRandomSeed();
    arduboy.clear();
    arduboy.setFrameRate(FRAMERATE);
}

void loop()
{
    if (!(arduboy.nextFrame())) return;

    arduboy.clear();
    render();
    movement();
    arduboy.display();
}
