#include <Tinyfont.h>
#include <FixedPoints.h>
#include <Arduboy2.h>

#include <math.h>

// Graphics
#include "resources/menu.h"
#include "resources/raycastbg.h"
#include "tile.h"

// Libs (sort of; mostly just code organization)
#include "utils.h"
#include "mazedef.h"
#include "mazegen.h"
#include "shading.h"

Arduboy2Base arduboy;
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());


// Corner shadows are slightly more expensive, but help visually 
// separate the darker walls (North/South) from the floor and gives a nice effect
#define CORNERSHADOWS

// You probably want the floor + whatever else that aren't walls
#define DRAWFOUNDATION

// Texture precision. 0 is lowest, 2 is highest. Each level down saves
// enough frames to matter
#define TEXPRECISION 2

// Choose a method for unsigned reciprocal of unit lengths (or close to it).
// 1: greatly increases speed but with visual artifacts and 512 extra program bytes
// 2: greatly increases speed without visual artifacts and 512 extra program bytes (marginally slower than 1; prefer 2)
// 3: very slow but saves the 512 bytes
//#define URCPLUNIT(x) uReciprocalUnit(x)
#define URCPLUNIT(x) uReciprocalNearUnit(x)
//#define URCPLUNIT(x) (1 / (x))


// And now some debug stuff
// #define DRAWMAPDEBUG     // Display map (will take up large portion of screen)
// #define LINEHEIGHTDEBUG  // Display information about lineheight (only draws a few lines)
// #define NOWALLSHADING    // Wall shading actually reduces the cost... I must have a bug
// #define FAKEFOV 0.8      // Not that useful, may break. 1 = 90, higher = more than 90


// Gameplay constants
constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;
constexpr uflot LIGHTINTENSITY = 1.5;

//These are calculated constants based off your light intensity. The view 
//distance is an optimization; increase light intensity to increase view distance.
//We calculate "darkness" to avoid 100 divisions per frame (huuuge savings)
const uflot VIEWDISTANCE = sqrt(BAYERGRADIENTS * (float)LIGHTINTENSITY);
const uflot DARKNESS = 1 / LIGHTINTENSITY;

//Screen calc constants (no need to do it at runtime)
constexpr int16_t MIDSCREEN = HEIGHT / 2;
constexpr uint8_t VIEWWIDTH = 100;
constexpr flot NORMWIDTH = 2.0f / VIEWWIDTH;
constexpr uint8_t BWIDTH = WIDTH >> 3;
constexpr uint8_t LDISTSAFE = 16;
constexpr uflot MINLDISTANCE = 1.0f / LDISTSAFE;
constexpr uint16_t MAXLHEIGHT = HEIGHT * LDISTSAFE;


//Menu related stuff
uint8_t menuIndex = 0;
uint8_t mazeSize = 0;
uint8_t mazeType = 0;

uint8_t curWidth = 0;
uint8_t curHeight = 0;

uint8_t totalWins = 1; //lol
float thisDistance = 0;
float totalDistance = 0;

// Position and facing direction
float posX, posY; 
float dirX, dirY;

//The map itself!
uint8_t worldMap[MAXMAPHEIGHT * MAXMAPWIDTH];

//uflot darkmod = 0;

// Full clear the raycast area. Not always used
void clearRaycast() 
{
    fastClear(&arduboy, 0, 0, VIEWWIDTH, HEIGHT);
}

//Draw the floor underneath the raycast walls (ultra simple for now to save cycles)
void raycastFoundation()
{
    // Actually changed it to a full bg
    Sprites::drawOverwrite(0, 0, raycastBg, 0);
}

// The full function for raycasting. 
void raycast()
{
    //darkmod = sin(arduboy.frameCount) * 0.1; // + 2) * 0.333;
    //Waste < 20 bytes of stack to save numerous cycles on render (and on programmer. leaving posX + posY floats so...)
    uint8_t pmapX = int(posX);
    uint8_t pmapY = int(posY);
    uflot pmapofsX = posX - pmapX;
    uflot pmapofsY = posY - pmapY;
    flot fposX = posX;
    flot fposY = posY;
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
        // never larger than 1 / NEARZEROFIXED on any side, it will be fine (that means
        // map has to be < 100 on a side with this)
        if(deltaDistX > NEARZEROFIXED) {
            deltaDistX = URCPLUNIT(deltaDistX);
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
            deltaDistY = URCPLUNIT(deltaDistY);
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
        uflot perpWallDist = 0;     // perpendicular distance (not real distance)
        uint8_t tile = TILEEMPTY;   // tile that was hit by ray

        // perform DDA. A do/while loop is ever-so-slightly faster it seems?
        do
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
            tile = getMazeCell(worldMap, mapX, mapY);
        }
        while (perpWallDist < VIEWDISTANCE && tile == TILEEMPTY);

        // If the above loop was exited without finding a tile, there's nothing to draw
        if(tile == TILEEMPTY) continue;

        //NOTE: wallX technically can only be positive, but I'm using flot to save a tiny amount from casting
        flot wallX = side ? fposX + (flot)perpWallDist * rayDirX : fposY + (flot)perpWallDist * rayDirY;
        wallX -= floorFixed(wallX); //.getFraction isn't working!
        uint8_t texX = uint8_t(wallX * TILESIZE);
        if((side == 0 && rayDirX > 0) || (side == 1 && rayDirY < 0)) texX = TILESIZE - 1 - texX;

        // Calculate half height of line to draw on screen. We already know the distance to the wall.
        // We can truncate the total height if too close to the wall right here and now and avoid future checks.
        uint16_t lineHeight = (perpWallDist <= MINLDISTANCE) ? MAXLHEIGHT : (HEIGHT / (float)perpWallDist);

        #ifdef LINEHEIGHTDEBUG
        tinyfont.setCursor(16, x * 16);
        tinyfont.println(lineHeight);
        tinyfont.setCursor(16, x * 16 + 8);
        tinyfont.println((float)perpWallDist);
        if(x > 2) break;
        #endif

        //ending should be exclusive
        draw_wall_line(x, lineHeight, perpWallDist, side, tile, texX);
    }
}

//Draw a single raycast wall line. Will only draw specifically the wall line and will clip out all the rest
//(so you can predraw a ceiling and floor before calling raycast)
inline void draw_wall_line(uint8_t x, uint16_t lineHeight, uflot distance, uint8_t side, uint8_t tile, uint8_t texX) 
{
    #ifdef NOWALLSHADING
    uint8_t shade = ((side & x) || tile == TILEEXIT) ? 0 : 0xFF;
    #else
    //NOTE: multiplication is WAY FASTER than division
    uint8_t dither = (uint8_t)(floorFixed(distance * DARKNESS * distance));
    //Oops, we're beyond dark (this shouldn't happen often but it CAN)
    if(dither >= BAYERGRADIENTS) return;
    uint8_t shade = (side & x) ? 0 : pgm_read_byte(b_shading + (dither * 4) + (x & 3));
    #endif

    int16_t halfLine = lineHeight >> 1;
    uint8_t yStart = max(0, MIDSCREEN - halfLine);
    uint8_t yEnd = min(HEIGHT, MIDSCREEN + halfLine);

    const uint8_t * tofs = wallTile + (tile - 1) * TILEBYTES + texX;
    uint16_t bofs = (yStart & 0b1111000) * BWIDTH + x;
    uint8_t texByte = arduboy.sBuffer[bofs];
    uint16_t texData = pgm_read_byte(tofs) + 256 * pgm_read_byte(tofs + TILESIZE);

    #if TEXPRECISION == 2
    UFixed<16,16> step = (float)TILESIZE / lineHeight;
    UFixed<16,16> texPos = (yStart + halfLine - MIDSCREEN) * step;
    #elif TEXPRECISION == 1
    UFixed<4,12> step = (float)TILESIZE / lineHeight;
    uflot tp1 = (yStart + halfLine - MIDSCREEN) * (uflot)step;
    UFixed<4,12> texPos = (UFixed<4,12>)tp1;
    #else
    uflot step = (float)TILESIZE / lineHeight;
    uflot texPos = (yStart + halfLine - MIDSCREEN) * step;
    #endif

    //Individual bits. Again, a do/while loop is slightly faster, it actually makes a difference here
    //over a for loop
    do
    {
        uint8_t bidx = yStart & 7;

        // Every new byte, save the current (previous) byte and load the new byte from the screen. 
        // This might be wasteful, as only the first and last byte technically need to pull from the screen. 
        if(bidx == 0) {
            arduboy.sBuffer[bofs] = texByte;
            bofs = (yStart & 0b1111000) * BWIDTH + x;
            texByte = arduboy.sBuffer[bofs];
        }

        uint8_t bm = fastlshift8(bidx);

        //Texture stuff goes here, after shade & bm (for shortcutting)
        if ((shade & bm) && (texData & fastlshift16(texPos.getInteger())))
            texByte |= bm;
        else
            texByte &= ~bm;

        texPos += step;
    }
    while(++yStart < yEnd);

    //Just in case, store the last one too
    #ifdef CORNERSHADOWS
    arduboy.sBuffer[bofs] = texByte & ~(fastlshift8((yEnd - 1) & 7));
    #else
    arduboy.sBuffer[bofs] = texByte;
    #endif
}

// Perform ONLY player movement updates! No drawing!
void movement()
{
    // move forward if no wall in front of you
    if (arduboy.pressed(A_BUTTON))
    {
        float movX = isCellSolid(worldMap, (int)(posX + dirX * MOVESPEED), (int)posY) ? 0 : dirX * MOVESPEED;
        float movY = isCellSolid(worldMap, (int)posX, (int)(posY + dirY * MOVESPEED)) ? 0 : dirY * MOVESPEED;
        thisDistance += sqrt(movX * movX + movY * movY);
        posX += movX;
        posY += movY;
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

inline bool inExit() { return getMazeCell(worldMap, (int)posX, (int)posY) == TILEEXIT; }

//Menu functionality, move the cursor, select things (redraws automatically)
void doMenu()
{
    constexpr uint8_t MENUITEMS = 4;
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

    Sprites::drawOverwrite(VIEWWIDTH, 0, menu, 0);
    tinyfont.setCursor(109, 4);
    tinyfont.print(F("3D"));
    tinyfont.setCursor(105, 9);
    tinyfont.print(F("MAZE"));

    MazeSize mzs = getMazeSize(mazeSize);
    tinyfont.setCursor(MENUX + 4, MENUY);
    tinyfont.print(mzs.name);

    MazeType mzt = getMazeType(mazeType);
    tinyfont.setCursor(MENUX + 4, MENUY + MENUSPACING);
    tinyfont.print(mzt.name);

    tinyfont.setCursor(MENUX + 4, MENUY + MENUSPACING * 2);
    tinyfont.print(F("NEW"));

    tinyfont.setCursor(MENUX + 4, MENUY + MENUSPACING * 3);
    tinyfont.print(F("HNT"));

    tinyfont.setCursor(MENUX, MENUY + menuIndex * MENUSPACING);
    tinyfont.print("o");

    if(showHint)
    {
        arduboy.drawRect(MENUX + 5, HEIGHT - 15, 10, 10, WHITE);
        arduboy.drawPixel(MENUX + 14, HEIGHT - 14, BLACK);
        arduboy.drawPixel(MENUX + 6 + (int)(posX / curWidth * 8), HEIGHT - 7 - (int)(posY / curHeight * 8), arduboy.frameCount & 0b10000 ? WHITE : BLACK);
    }
}

// Generate a new maze and reset the game to an initial playable state
void generateMaze()
{
    clearRaycast();
    tinyfont.setCursor(12, 28);
    tinyfont.print(F("Generating maze"));
    arduboy.display();

    //Why am I doing this? It's mostly a meme I guess; don't write code like this!
    if(inExit())
        totalWins += 1;

    //Regardless if you win or not, put distance away
    totalDistance += thisDistance;
    thisDistance = 0;

    MazeSize mzs = getMazeSize(mazeSize);
    MazeType mzt = getMazeType(mazeType); 

    //Call the generator function chosen by the menu
    mzt.func(worldMap, mzs.width, mzs.height, &posX, &posY, &dirX, &dirY);
    curWidth = mzs.width;
    curHeight = mzs.height;
}


void setup()
{
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.flashlight();
    arduboy.initRandomSeed();
    arduboy.clear();
    arduboy.setFrameRate(FRAMERATE);
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
        clearRaycast();

        constexpr uint8_t WINX = 22;
        constexpr uint8_t WINY = 32;

        tinyfont.setCursor(WINX, 24);
        tinyfont.print(F("COMPLETE!"));
        tinyfont.setCursor(WINX + 8, WINY);
        tinyfont.print(F("WINS: "));
        tinyfont.print(totalWins);
        tinyfont.setCursor(WINX + 8, WINY + 5);
        tinyfont.print("DIST: ");
        tinyfont.print((int)thisDistance);
        tinyfont.setCursor(WINX + 3, WINY + 10);
        tinyfont.print("TDIST: ");
        tinyfont.print((int)(totalDistance + thisDistance));
    }
    else
    {
        #ifdef DRAWFOUNDATION
        raycastFoundation();
        #else
        clearRaycast();
        #endif

        raycast();
        movement();

        #ifdef DRAWMAPDEBUG
        drawMaze(&arduboy, worldMap, 0, 0);
        #endif
    }

    arduboy.display();
}
