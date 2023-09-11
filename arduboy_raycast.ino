#include <Arduboy2.h>
#include <math.h>
#include <float.h>

Arduboy2 arduboy;

//int thing = 0b101010;

constexpr uint8_t MIDSCREEN = HEIGHT / 2;

constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 6.0f / FRAMERATE;
constexpr float ROTSPEED = 4.0f / FRAMERATE;
constexpr uint8_t VIEWDISTANCE = 8;
// constexpr uint8_t MAXDDA = 10;

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

constexpr uint8_t sp_shading[] PROGMEM = { 
    1, 64,
    255,255,255,255,255,255,255,255,    // Full fill
    85,85,85,85,85,85,85,85,            // Half fill
    170,170,170,170,170,170,170,170,    // Half fill alt
};
//constexpr uint8_t sp_fill[8] = { 255,255,255,255,255,255,255,255 };
//constexpr uint8_t sp_half[8] = { 85,85,85,85,85,85,85,85 };
//constexpr uint8_t sp_half2[8] = { 170,170,170,170,170,170,170,170 };

float posX = 22, posY = 12;  //x and y start position
float dirX = -1, dirY = 0; //initial direction vector
//float dirX = 0, dirY = 1; //initial direction vector
float planeX = 0, planeY = 0.66; //the 2d raycaster version of camera plane

inline void render()
{
    for (uint8_t x = 0; x < WIDTH; x++)
    {
        // calculate ray position and direction
        float cameraX = (x << 1) / (float)WIDTH - 1; // x-coordinate in camera space
        float rayDirX = dirX + planeX * cameraX;
        float rayDirY = dirY + planeY * cameraX;
        // which box of the map we're in
        uint8_t mapX = int(posX);
        uint8_t mapY = int(posY);

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
        float deltaDistX = (rayDirX == 0) ? FLT_MAX : abs(1 / rayDirX);
        float deltaDistY = (rayDirY == 0) ? FLT_MAX : abs(1 / rayDirY);


        // length of ray from current position to next x or y-side
        float sideDistX;
        float sideDistY;

        // what direction to step in x or y-direction (either +1 or -1)
        int8_t stepX;
        int8_t stepY;

        // calculate step and initial sideDist
        if (rayDirX < 0) {
            stepX = -1;
            sideDistX = (posX - mapX) * deltaDistX;
        }
        else {
            stepX = 1;
            sideDistX = (mapX + 1.0 - posX) * deltaDistX;
        }
        if (rayDirY < 0) {
            stepY = -1;
            sideDistY = (posY - mapY) * deltaDistY;
        }
        else {
            stepY = 1;
            sideDistY = (mapY + 1.0 - posY) * deltaDistY;
        }

        float perpWallDist = 0;
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
            if (worldMap[mapX][mapY] > 0)
                break;
        }

        if(perpWallDist >= VIEWDISTANCE) continue;

        //Choose wall color based sort of on distance + wall side
        uint8_t color_offset = 0;

        // Calculate distance projected on camera direction. This is the shortest distance from the point where the wall is
        // hit to the camera plane. Euclidean to center camera point would give fisheye effect!
        // This can be computed as (mapX - posX + (1 - stepX) / 2) / rayDirX for side == 0, or same formula with Y
        // for size == 1, but can be simplified to the code below thanks to how sideDist and deltaDist are computed:
        // because they were left scaled to |rayDir|. sideDist is the entire length of the ray above after the multiple
        // steps, but we subtract deltaDist once because one step more into the wall was taken above.
        if (side == 1) {
            color_offset = 1;
        }

        // Calculate half height of line to draw on screen
        uint8_t lineHeight = (perpWallDist < 1 ? HEIGHT : (int)(HEIGHT / perpWallDist)) >> 1;

        // calculate lowest and highest pixel to fill in current stripe
        uint8_t drawStart = MIDSCREEN - lineHeight;
        uint8_t drawEnd = MIDSCREEN + lineHeight;

        Sprites::drawOverwrite(x, drawStart, sp_shading, color_offset);
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
