#include <Tinyfont.h>
#include <FixedPoints.h>
#include <Arduboy2.h>

#include <math.h>

// Libs (sort of; mostly just code organization)
#include "utils.h"
#include "rcmap.h"
#include "mazegen.h"
#include "shading.h"

// Graphics
#include "resources/menu.h"
#include "resources/raycastbg.h"
#include "sprites.h"
#include "tiles.h"


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

// Sprite precision. Only options are 2 and 0 (all else are 0). Note: I haven't 
// been able to see a difference, so I set it to 0
#define SPRITEPRECISION 0


// And now some debug stuff
// #define DRAWMAPDEBUG         // Display map (will take up portion of screen)
// #define LINEHEIGHTDEBUG      // Display information about lineheight (only draws a few lines)
// #define NOWALLSHADING        // Wall shading actually reduces the cost... I must have a bug
// #define NOSPRITES            // Remove all sprites
// #define ADDDEBUGSPRITES 4    // How many debug sprites to add at generation
// #define PRINTSPRITEDATA  // Having trouble with sprites sometimes


// Gameplay constants
constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;
constexpr uflot LIGHTINTENSITY = 1.5;
constexpr uflot FAKEFOV = 1.0;      // Not that useful, may break. 1 = 90, higher = more than 90

//These are calculated constants based off your light intensity. The view 
//distance is an optimization; increase light intensity to increase view distance.
//We calculate "darkness" to avoid 100 divisions per frame (huuuge savings)
const uflot VIEWDISTANCE = sqrt(BAYERGRADIENTS * (float)LIGHTINTENSITY);
const uflot DARKNESS = 1 / LIGHTINTENSITY;

//Screen calc constants (no need to do it at runtime)
constexpr uint8_t VIEWWIDTH = 100;
constexpr uint8_t MIDSCREENY = HEIGHT / 2;
constexpr uint8_t MIDSCREENX = VIEWWIDTH / 2;
constexpr uint8_t BWIDTH = WIDTH >> 3;

//Distance-based stuff
constexpr uint8_t LDISTSAFE = 16;
constexpr uflot MINLDISTANCE = 1.0f / LDISTSAFE;
constexpr uint16_t MAXLHEIGHT = HEIGHT * LDISTSAFE;
constexpr flot MINSPRITEDISTANCE = 0.2;

// Size limit for data structures
constexpr uint8_t NUMSPRITES = 32;
constexpr uint8_t MAPWIDTH = 16;
constexpr uint8_t MAPHEIGHT = 16;



//Menu related stuff
uint8_t menuIndex = 0;
uint8_t mazeSize = 1;
uint8_t mazeType = 0;

uint8_t curWidth = 0;
uint8_t curHeight = 0;

uint8_t totalWins = 1; //lol
float thisDistance = 0;
uint16_t totalDistance = 0;

// Position and facing direction
uflot posX, posY;
float dirX, dirY; //These HAVE TO be float, or something with a lot more precision

// These are bitmasks to get data out of state
constexpr uint8_t RSSTATEACTIVE = 0b00000001;
constexpr uint8_t RSSTATESHRINK = 0b00000110;
constexpr uint8_t RSTATEYOFFSET = 0b11111000;

// Try to make this fit into as little space as possible
struct RSprite {
    muflot x; //Precision for x/y is low but doesn't really need to be high
    muflot y;
    uint8_t frame = 0;
    uint8_t state = 0; // First bit is active, next 2 are how many times to /2 for size

    //Note: if we sacrifice 2 bytes per sprite to store the distance (so 32 bytes currently), 
    //you can potentially save computation by sorting the sprite list itself. Sorted lists 
    //generally don't require as much computation. I don't know realistically how much is 
    //being saved though, so I'm not using that. I think having the sort struct below is
    //significantly more flexible and may actually end up being faster overall
};

// Sorted sprite. Needed to save memory in RSprite (distance). May add more data here
// for precomputation
struct SSprite {
    RSprite * sprite; 
    UFixed<12,4> distance;    //Unfortunately, distance kinda has to be large... 12 bits = 4096, should be more than enough
};

constexpr uint8_t MAZETYPECOUNT = 2;
constexpr MazeType MAZETYPES[MAZETYPECOUNT] PROGMEM = {
    { "MAZ", &genMazeType },
    { "EPT", &genSparseRandom },
    //{ "BKR", &genRoomsType }, // I may add more later? (2023-09-14 for the lols)
    //{ "CEL", &genCellType }
};

constexpr uint8_t MAZESIZECOUNT = 2;
constexpr MazeSize MAZESIZES[MAZESIZECOUNT] PROGMEM = {
    { "SML", 11, 11 },  //NOTE: must always be 2N + 1
    { "MED", 15, 15 },
    //{ "LRG", 15, 15},
    //{ "XL ", 47, 47 },
    // { "XXL", 60, 60 } //Only if we have room (we don't)
};


//Big data!
uint8_t mapBuffer[MAPHEIGHT * MAPWIDTH];
RcMap worldMap {
    mapBuffer,
    MAPWIDTH,
    MAPHEIGHT
};
uflot distCache[VIEWWIDTH / 2]; // Half distance resolution means sprites will clip 1 pixel into walls sometimes but otherwise...
RSprite sprites[NUMSPRITES];


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
    //Waste ~20 bytes of stack to save numerous cycles on render (and on programmer. leaving posX + posY floats so...)
    uint8_t pmapX = posX.getInteger();
    uint8_t pmapY = posY.getInteger();
    uflot pmapofsX = posX - pmapX;
    uflot pmapofsY = posY - pmapY;
    flot fposX = (flot)posX, fposY = (flot)posY;
    flot dX = dirX, dY = dirY;
    flot planeX = dY * (flot)FAKEFOV, planeY = - dX * (flot)FAKEFOV; // Camera vector or something, simple -90 degree rotate from dir
    constexpr flot INVWIDTH = 2.0f / VIEWWIDTH;

    for (uint8_t x = 0; x < VIEWWIDTH; x++)
    {
        flot cameraX = x * INVWIDTH - 1; // x-coordinate in camera space

        // The camera plane is a simple -90 degree rotation on the player direction (as required for this algorithm).
        flot rayDirX = dX + planeX * cameraX;
        flot rayDirY = dY + planeY * cameraX;

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
            deltaDistX = uReciprocalNearUnit(deltaDistX); 
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
            deltaDistY = uReciprocalNearUnit(deltaDistY); 
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
            tile = getMapCell(&worldMap, mapX, mapY);
        }
        while (perpWallDist < VIEWDISTANCE && tile == TILEEMPTY);

        distCache[x >> 1] = perpWallDist;

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
    // ------- BEGIN CRITICAL SECTION -------------
    #ifdef NOWALLSHADING
    uint8_t shade = ((side & x) || tile == TILEEXIT) ? 0 : 0xFF;
    #else
    //NOTE: multiplication is WAY FASTER than division, hence "darkness" value instead of light
    uint8_t dither = (uint8_t)(floorFixed(distance * DARKNESS * distance));
    //Oops, we're beyond dark (this shouldn't happen often but it CAN)
    if(dither >= BAYERGRADIENTS) return;
    uint8_t shade = (side & x) ? 0 : pgm_read_byte(b_shading + (dither * 4) + (x & 3));
    #endif

    int16_t halfLine = lineHeight >> 1;
    uint8_t yStart = max(0, MIDSCREENY - halfLine);
    uint8_t yEnd = min(HEIGHT, MIDSCREENY + halfLine) - 1;

    uint16_t bofs = (yStart & 0b1111000) * BWIDTH + x;
    uint8_t texByte = arduboy.sBuffer[bofs];
    uint16_t texData = readTextureStrip16(tilesheet, tile - 1, texX);

    #if TEXPRECISION == 2
    UFixed<16,16> step = (float)TILESIZE / lineHeight;
    UFixed<16,16> texPos = (yStart + halfLine - MIDSCREENY) * step;
    #elif TEXPRECISION == 1
    UFixed<4,12> step = (float)TILESIZE / lineHeight;
    uflot tp1 = (yStart + halfLine - MIDSCREENY) * (uflot)step;
    UFixed<4,12> texPos = (UFixed<4,12>)tp1;
    #else
    uflot step = (float)TILESIZE / lineHeight;
    uflot texPos = (yStart + halfLine - MIDSCREENY) * step;
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
    while(++yStart <= yEnd);

    //Just in case, store the last one too
    #ifdef CORNERSHADOWS
    arduboy.sBuffer[bofs] = texByte & ~(fastlshift8(yEnd & 7));
    #else
    arduboy.sBuffer[bofs] = texByte;
    #endif
    // ------- END CRITICAL SECTION -------------
}

void drawSprites()
{
    flot fposX = (flot)posX, fposY = (flot)posY;
    flot dX = dirX, dY = dirY;
    flot planeX = dY * (flot)FAKEFOV, planeY = - dX * (flot)FAKEFOV; // Camera vector or something, simple -90 degree rotate from dir

    //Make a temp sort array on stack
    uint8_t usedSprites = 0;
    SSprite sorted[NUMSPRITES];

    // Calc distance. Also, sort elements (might as well, we're already here)
    for (uint8_t i = 0; i < NUMSPRITES; ++i)
    {
        if (!(sprites[i].state & 1))
            continue;

        flot sx = (flot)sprites[i].x - fposX;
        flot sy = (flot)sprites[i].y - fposY;

        SSprite toSort;
        toSort.distance = UFixed<12, 4>(sx * sx) + UFixed<12,4>(sy * sy); // sqrt not taken, unneeded
        toSort.sprite = &sprites[i];

        //Insertion sort (it's faster for small arrays; if you increase sprite count to some 
        //absurd number, change this to something else).
        int8_t insertPos = usedSprites - 1;

        while(insertPos >= 0 && sorted[insertPos].distance < toSort.distance)
        {
            sorted[insertPos + 1] = sorted[insertPos];
            insertPos--;
        }

        sorted[insertPos + 1] = toSort;
        usedSprites++;
    }

    //Some matrix math stuff
    flot invDet = 1.0 / (planeX * dY - planeY * dX); // required for correct matrix multiplication

    // after sorting the sprites, do the projection and draw them. We know all sprites in the array are active,
    // since we're looping against the sorted array.
    for (uint8_t i = 0; i < usedSprites; i++)
    {
        //Get the current sprite. Copy so we don't have to derefence a pointer a million times
        RSprite sprite = * sorted[i].sprite;

        // translate sprite position to relative to camera (we calculated this before, but need to save mem)
        flot spriteX = (flot)sprite.x - fposX;
        flot spriteY = (flot)sprite.y - fposY;

        // X and Y will always be very small (map only 4 bit size), so these transforms will still fit within a 7 bit int part
        flot transformYT = invDet * (-planeY * spriteX + planeX * spriteY); // this is actually the depth inside the screen, that what Z is in 3D

        // Nice quick shortcut to get out for sprites behind us (and ones that are too close)
        if(transformYT < MINSPRITEDISTANCE) continue;

        uflot transformY = (uflot)transformYT; //Need this as uflot for critical loop
        flot transformX = invDet * (dY * spriteX - dX * spriteY);

        //int16 because easy overflow! if x is much larger than y, then you're effectively multiplying 50 by map width.
        // NOTE: this is the CENTER of the sprite, not the edge (thankfully)
        int16_t spriteScreenX = int16_t(MIDSCREENX * (1 + (float)transformX / (float)transformYT));

        // calculate the dimensions of the sprite on screen. All sprites are square. Size mods go here
        // using 'transformY' instead of the real distance prevents fisheye
        uint16_t spriteHeight = uint16_t(HEIGHT / (float)transformY) >> ((sprite.state & RSSTATESHRINK) >> 1); 
        uint16_t spriteWidth = spriteHeight; 

        // calculate lowest and highest pixel to fill. Sprite screen/start X and Sprite screen/start Y
        // Because we have 1 fewer bit to store things, we unfortunately need an int16
        int16_t ssX = -(spriteWidth >> 1) + spriteScreenX;   //Offsets go here, but modified by distance or something?
        int16_t ssXe = ssX + spriteWidth;

        // Get out if sprite is completely outside view
        if(ssXe < 0 || ssX >= VIEWWIDTH) continue;

        //Calculate vMove from top 5 bits of state
        uint8_t yShiftBits = sprite.state >> 3;
        int8_t yShift = yShiftBits ? int8_t((yShiftBits & 16 ? -(yShiftBits & 15) : yShiftBits) * 2.0 / transformY) : 0;
        //The above didn't work without float math, didn't feel like figuring out the ridiculous type casting

        int16_t ssY = -(spriteHeight >> 1) + MIDSCREENY + yShift;
        int16_t ssYe = ssY + spriteHeight;

        uint8_t drawStartY = ssY < 0 ? 0 : ssY; //Because of these checks, we can store them in 1 byte stuctures
        uint8_t drawEndY = ssYe >= HEIGHT ? HEIGHT - 1 : ssYe;
        uint8_t drawStartX = ssX < 0 ? 0 : ssX;
        uint8_t drawEndX = ssXe >= VIEWWIDTH ? VIEWWIDTH - 1 : ssXe;

        //Setup stepping to avoid costly mult (and div) in critical loops
        //These float divisions happen just once per sprite, hopefully that's not too bad
        #if SPRITEPRECISION == 2
        UFixed<16,16> stepX = (float)TILESIZE / spriteWidth;
        UFixed<16,16> stepY = (float)TILESIZE / spriteHeight;
        UFixed<16,16> texX = (drawStartX - ssX) * stepX;
        UFixed<16,16> texYInit = (drawStartY - ssY) * stepY;
        UFixed<16,16> texY = texYInit;
        #else
        uflot stepX = (float)TILESIZE / spriteWidth;
        uflot stepY = (float)TILESIZE / spriteHeight;
        uflot texX = (drawStartX - ssX) * stepX;
        uflot texYInit = (drawStartY - ssY) * stepY;
        uflot texY = texYInit;
        #endif

        uint8_t fr = sprites[i].frame;
        uint8_t x = drawStartX;

        // loop through every vertical stripe of the sprite on screen
        // ------- BEGIN CRITICAL SECTION -------------
        do
        {
            //If the sprite is hidden, most processing disappears
            if (transformY >= distCache[x >> 1]) continue;

            uint8_t tx = texX.getInteger();
            uint16_t texData = readTextureStrip16(spritesheet, fr, tx);
            uint16_t texMask = readTextureStrip16(spritesheet_Mask, fr, tx);

            uflot texY = texYInit;
            uint8_t y = drawStartY;

            uint16_t bofs = (y & 0b1111000) * BWIDTH + x;
            uint8_t texByte = arduboy.sBuffer[bofs];

            do //For every pixel of the current sprite
            {
                uint8_t bidx = y & 7;

                // Every new byte, save the current (previous) byte and load the new byte from the screen. 
                // This might be wasteful, as only the first and last byte technically need to pull from the screen. 
                if(bidx == 0) {
                    arduboy.sBuffer[bofs] = texByte;
                    bofs = (y & 0b1111000) * BWIDTH + x;
                    texByte = arduboy.sBuffer[bofs];
                }

                uint16_t mask = fastlshift16(texY.getInteger());

                if(texMask & mask)
                {
                    uint8_t bm = fastlshift8(bidx);

                    if ((texData & mask))
                        texByte |= bm;
                    else
                        texByte &= ~bm;
                }

                texY += stepY;
            }
            while(++y < drawEndY);

            arduboy.sBuffer[bofs] = texByte;
            texX += stepX;
        }
        while(++x < drawEndX);
        // ------- END CRITICAL SECTION -------------

        #ifdef PRINTSPRITEDATA
        //Clear a section for us to use
        constexpr uint8_t sdh = 10;
        arduboy.fillRect(0, HEIGHT - sdh, VIEWWIDTH, sdh, BLACK);
        //Print some junk
        tinyfont.setCursor(0, HEIGHT - sdh);
        tinyfont.print((float)transformX, 4);
        tinyfont.print(" X");
        tinyfont.print(ssX);
        tinyfont.setCursor(0, HEIGHT - sdh + 5);
        tinyfont.print((float)transformY, 4);
        tinyfont.print(" W");
        tinyfont.print(spriteWidth);
        #endif

    }
}

// Perform ONLY player movement updates! No drawing!
void movement()
{
    // move forward if no wall in front of you
    if (arduboy.pressed(A_BUTTON))
    {
        float movX = dirX * MOVESPEED;
        float movY = dirY * MOVESPEED;

        if(isCellSolid(&worldMap, ((flot)posX + movX).getInteger(), posY.getInteger())) movX = 0;
        if(isCellSolid(&worldMap, posX.getInteger(), (int)((flot)posY + movY))) movY = 0;

        thisDistance += sqrt((movX * movX + movY * movY));

        posX = uflot((flot)posX + movX);
        posY = uflot((flot)posY + movY);
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

inline bool inExit() { return getMapCell(&worldMap, (int)posX, (int)posY) == TILEEXIT; }

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

    MazeSize mzs = getMazeSize(MAZESIZES, mazeSize);
    tinyfont.setCursor(MENUX + 4, MENUY);
    tinyfont.print(mzs.name);

    MazeType mzt = getMazeType(MAZETYPES, mazeType);
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

    MazeSize mzs = getMazeSize(MAZESIZES, mazeSize);
    MazeType mzt = getMazeType(MAZETYPES, mazeType); 

    //Call the generator function chosen by the menu
    mzt.func(&worldMap, mzs.width, mzs.height, &posX, &posY, &dirX, &dirY);
    curWidth = mzs.width;
    curHeight = mzs.height;

    #ifdef ADDDEBUGSPRITES
    for(uint8_t i = 0; i < ADDDEBUGSPRITES; i++)
    {
        sprites[i].x = muflot((flot)posX + dirX * (1 + i));
        sprites[i].y = muflot((flot)posY + dirY * (1 + i));
        sprites[i].frame = 0;
        sprites[i].state = 1 | (i << 1) | ((i * 4) << 3); //active + shrink
    }
    #endif
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
        #ifndef NOSPRITES
        drawSprites();
        #endif
        movement();

        #ifdef DRAWMAPDEBUG
        drawMaze(&arduboy, worldMap, 0, 0);
        #endif
    }

    arduboy.display();
}
