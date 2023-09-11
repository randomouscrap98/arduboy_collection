#include <FixedPoints.h>

#include <Arduboy2.h>
#include <math.h>
#include <float.h>

Arduboy2 arduboy;

//int thing = 0b101010;

constexpr uint8_t MIDSCREEN = HEIGHT / 2;

constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 5.0f / FRAMERATE;
constexpr float ROTSPEED = 4.0f / FRAMERATE;
constexpr uint8_t VIEWDISTANCE = 8;

constexpr uint8_t MAPWIDTH = 24;
constexpr uint8_t MAPHEIGHT = 24;

uint8_t worldMap[MAPWIDTH][MAPHEIGHT]=
{
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

typedef SFixed<7,8> flot;
typedef UFixed<8,8> uflot;
constexpr uint8_t MAXFIXED = 255;
constexpr uflot NEARZEROFIXED = 1 / MAXFIXED;

inline void render()
{
    uint8_t pmapX = int(posX);
    uint8_t pmapY = int(posY);
    uflot pmapofsX = posX - pmapX;
    uflot pmapofsY = posY - pmapY;

    for (uint8_t x = 0; x < WIDTH; x++)
    {
        // calculate ray position and direction
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
        uflot deltaDistX = (uflot)abs(rayDirX); //(rayDirX == 0) ? 1e10 : abs(1 / rayDirX);
        uflot deltaDistY = (uflot)abs(rayDirY); //(rayDirY == 0) ? 1e10 : abs(1 / rayDirY);

        // length of ray from current position to next x or y-side
        uflot sideDistX = MAXFIXED;
        uflot sideDistY = MAXFIXED;

        // what direction to step in x or y-direction (either +1 or -1)
        int8_t stepX = 0;
        int8_t stepY = 0;

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
                perpWallDist = sideDistX;
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
        uint8_t color_offset = (int)(perpWallDist / 2);

        // Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
        // hit to the camera plane. Euclidean to center camera point would give fisheye effect!
        // This can be computed as (mapX - posX + (1 - stepX) / 2) / rayDirX for side == 0, or same formula with Y
        // for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
        // because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
        // steps, but we subtract deltaDist once because one step more into the wall was taken above.
        //if (side == 1) 
        //    color_offset += 1;

        if(color_offset >= GRADIENTS)
            color_offset = GRADIENTS - 1;

        // Calculate half height of line to draw on screen
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

    //arduboy.setCursor(0, 0);

    //char thing[20];
    //sprintf(thing, "Byte: %d", sizeof(uint8_t));
    //arduboy.println(thing);
    //sprintf(thing, "Int: %d", sizeof(int));
    //arduboy.println(thing);
    //sprintf(thing, "Float: %d", sizeof(float));
    //arduboy.println(thing);
    //// print("Int: %d")

    //arduboy.display();
}
