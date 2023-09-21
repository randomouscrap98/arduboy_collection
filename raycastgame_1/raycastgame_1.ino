#include <Tinyfont.h>
#include <FixedPoints.h>
#include <Arduboy2.h>

// Libs (sort of; mostly just code organization)
#include "utils.h"
#include "rcmap.h"
#include "mazegen.h"
#include "rcsprite.h"
#include "behaviors.h"

#include "raycast.h"

// Graphics
#include "resources/raycastbg.h"
#include "spritesheet.h"
#include "tilesheet.h"

//ARDUBOY_NO_USB

Arduboy2Base arduboy;
Tinyfont tinyfont = Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

// And now some debug stuff
// #define DRAWMAPDEBUG         // Display map (will take up portion of screen)
// #define NOSPRITES            // Remove all sprites
 #define ADDDEBUGAREA     // Add a little debug area
//  #define PRINTSPRITEDATA  // Having trouble with sprites sometimes

// Gameplay constants
constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 3.5f / FRAMERATE;
constexpr float ROTSPEED = 3.5f / FRAMERATE;

//Distance-based stuff
constexpr float MINSPRITEDISTANCE = 0.2;

// Size limit for data structures
constexpr uint8_t NUMSPRITES = 32;
constexpr uint8_t NUMBOUNDS = 16;
constexpr uint8_t MAPWIDTH = 16;
constexpr uint8_t MAPHEIGHT = 16;


//Menu related stuff
uint8_t menuIndex = 0;
uint8_t mazeSize = 1;
uint8_t mazeType = 0;

uint8_t curWidth = 0;
uint8_t curHeight = 0;

constexpr uint8_t MAZETYPECOUNT = 2;
constexpr MazeType MAZETYPES[MAZETYPECOUNT] PROGMEM = {
    { "MAZ", &genMazeType },
    { "EPT", &genSparseRandom },
};

constexpr uint8_t MAZESIZECOUNT = 2;
constexpr MazeSize MAZESIZES[MAZESIZECOUNT] PROGMEM = {
    { "SML", 11, 11 },  //NOTE: must always be 2N + 1
    { "MED", 15, 15 },
};


//Big data!
uint8_t mapBuffer[MAPHEIGHT * MAPWIDTH];
RcMap worldMap {
    mapBuffer,
    MAPWIDTH,
    MAPHEIGHT
};
RcPlayer player { };
RSprite sprites[NUMSPRITES];
RBounds bounds[NUMBOUNDS];


//Draw the floor underneath the raycast walls (ultra simple for now to save cycles)
void raycastFoundation()
{
    // Actually changed it to a full bg
    Sprites::drawOverwrite(0, 0, raycastBg, 0);
    //raycastFloor();
}


void drawSprites()
{
    SFixed<11,4> fposx = (SFixed<11,4>)player.posX;
    SFixed<11,4> fposy = (SFixed<11,4>)player.posY;

    //Make a temp sort array on stack
    uint8_t usedSprites = 0;
    SSprite sorted[NUMSPRITES];

    // Calc distance. Also, sort elements (might as well, we're already here)
    for (uint8_t i = 0; i < NUMSPRITES; ++i)
    {
        if (!(sprites[i].state & 1))
            continue;

        SSprite toSort;
        toSort.dpx = (SFixed<11,4>)sprites[i].x - fposx;
        toSort.dpy = (SFixed<11,4>)sprites[i].y - fposy;
        toSort.distance = toSort.dpx * toSort.dpx + toSort.dpy * toSort.dpy; // sqrt not taken, unneeded
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

    float planeX = player.dirY, planeY = -player.dirX;
    float invDet = 1.0 / (planeX * player.dirY - planeY * player.dirX); // required for correct matrix multiplication

    // after sorting the sprites, do the projection and draw them. We know all sprites in the array are active,
    // since we're looping against the sorted array.
    for (uint8_t i = 0; i < usedSprites; i++)
    {
        //Get the current sprite. Copy so we don't have to derefence a pointer a million times
        RSprite sprite = * sorted[i].sprite;

        //Already stored pos relative to camera earlier, but want extra precision, use floats
        float spriteX = float(sorted[i].dpx);
        float spriteY = float(sorted[i].dpy);

        // X and Y will always be very small (map only 4 bit size), so these transforms will still fit within a 7 bit int part
        float transformYT = invDet * (-planeY * spriteX + planeX * spriteY); // this is actually the depth inside the screen, that what Z is in 3D

        // Nice quick shortcut to get out for sprites behind us (and ones that are too close)
        if(transformYT < MINSPRITEDISTANCE) continue;

        float transformXT = invDet * (player.dirY * spriteX - player.dirX * spriteY);

        //int16 because easy overflow! if x is much larger than y, then you're effectively multiplying 50 by map width.
        // NOTE: this is the CENTER of the sprite, not the edge (thankfully)
        int16_t spriteScreenX = int16_t(MIDSCREENX * (1 + transformXT / transformYT));

        // calculate the dimensions of the sprite on screen. All sprites are square. Size mods go here
        // using 'transformY' instead of the real distance prevents fisheye
        uint16_t spriteHeight = uint16_t(HEIGHT / transformYT) >> ((sprite.state & RSSTATESHRINK) >> 1); 
        uint16_t spriteWidth = spriteHeight; 

        // calculate lowest and highest pixel to fill. Sprite screen/start X and Sprite screen/start Y
        // Because we have 1 fewer bit to store things, we unfortunately need an int16
        int16_t ssX = -(spriteWidth >> 1) + spriteScreenX;   //Offsets go here, but modified by distance or something?
        int16_t ssXe = ssX + spriteWidth; //EXCLUSIVE

        // Get out if sprite is completely outside view
        if(ssXe < 0 || ssX > VIEWWIDTH) continue;

        //Calculate vMove from top 5 bits of state
        uint8_t yShiftBits = sprite.state >> 3;
        int8_t yShift = yShiftBits ? int8_t((yShiftBits & 16 ? -(yShiftBits & 15) : yShiftBits) * 2.0 / transformYT) : 0;
        //The above didn't work without float math, didn't feel like figuring out the ridiculous type casting

        int16_t ssY = -(spriteHeight >> 1) + MIDSCREENY + yShift;
        int16_t ssYe = ssY + spriteHeight; //EXCLUSIVE

        if(ssYe < 0 || ssY > HEIGHT) continue;

        uint8_t drawStartY = ssY < 0 ? 0 : ssY; //Because of these checks, we can store them in 1 byte stuctures
        uint8_t drawEndY = ssYe > HEIGHT ? HEIGHT : ssYe;
        uint8_t drawStartX = ssX < 0 ? 0 : ssX;
        uint8_t drawEndX = ssXe > VIEWWIDTH ? VIEWWIDTH : ssXe;

        //Setup stepping to avoid costly mult (and div) in critical loops
        //These float divisions happen just once per sprite, hopefully that's not too bad.
        //There used to be an option to set the precision of sprites but it didn't seem to make any difference
        uflot stepX = (float)TILESIZE / spriteWidth;
        uflot stepY = (float)TILESIZE / spriteHeight;
        uflot texX = (drawStartX - ssX) * stepX;
        uflot texYInit = (drawStartY - ssY) * stepY;
        uflot texY = texYInit;

        uflot transformY = (uflot)transformYT; //Need this as uflot for critical loop
        uint8_t fr = sprite.frame;
        uint8_t x = drawStartX;

        // ------- BEGIN CRITICAL SECTION -------------
        do //For every strip (x)
        {
            //If the sprite is hidden, most processing disappears
            if (transformY < distCache[x >> 1])
            {
                uint8_t tx = texX.getInteger();

                texY = texYInit;

                //These five variables are needed as part of the loop unrolling system
                uint16_t bofs;
                uint8_t texByte;
                uint16_t texData = readTextureStrip16(spritesheet, fr, tx);
                uint16_t texMask = readTextureStrip16(spritesheet_Mask, fr, tx);
                uint8_t thisWallByte = (((drawStartY >> 1) >> 1) >> 1); //right shift 3 without loop

                //Pull screen byte, save location
                #define _SPRITEREADSCRBYTE() bofs = thisWallByte * WIDTH + x; texByte = arduboy.sBuffer[bofs];
                //Write previously read screen byte, go to next byte
                #define _SPRITEWRITESCRNEXT() arduboy.sBuffer[bofs] = texByte; thisWallByte++;
                //Work for setting bits of screen byte
                #define _SPRITEBITUNROLL(bm,nbm) { uint16_t btmask = fastlshift16(texY.getInteger()); if (texMask & btmask) { if (texData & btmask) texByte |= bm; else texByte &= nbm; } texY += stepY; }

                _SPRITEREADSCRBYTE();

                #ifdef CRITICALLOOPUNROLLING

                uint8_t startByte = thisWallByte; //The byte within which we start, always inclusive
                uint8_t endByte = (((drawEndY >> 1) >> 1) >> 1);  //The byte to end the unrolled loop on. Could be inclusive or exclusive

                //First and last bytes are tricky
                if(drawStartY & 7)
                {
                    uint8_t endFirst = min((startByte + 1) * 8, drawEndY);

                    for (uint8_t i = drawStartY; i < endFirst; i++)
                    {
                        uint8_t bm = fastlshift8(i & 7);
                        _SPRITEBITUNROLL(bm, (~bm));
                    }

                    //Move to next, like it never happened
                    _SPRITEWRITESCRNEXT();
                    _SPRITEREADSCRBYTE();
                }

                //Now the unrolled loop
                while(thisWallByte < endByte)
                {
                    _SPRITEBITUNROLL(0b00000001, 0b11111110);
                    _SPRITEBITUNROLL(0b00000010, 0b11111101);
                    _SPRITEBITUNROLL(0b00000100, 0b11111011);
                    _SPRITEBITUNROLL(0b00001000, 0b11110111);
                    _SPRITEBITUNROLL(0b00010000, 0b11101111);
                    _SPRITEBITUNROLL(0b00100000, 0b11011111);
                    _SPRITEBITUNROLL(0b01000000, 0b10111111);
                    _SPRITEBITUNROLL(0b10000000, 0b01111111);
                    _SPRITEWRITESCRNEXT();
                    _SPRITEREADSCRBYTE();
                }

                //Last byte, but only need to do it if we end in the middle of a byte and don't simply span one byte
                if((drawEndY & 7) && startByte != endByte)
                {
                    for (uint8_t i = thisWallByte * 8; i < drawEndY; i++)
                    {
                        uint8_t bm = fastlshift8(i & 7);
                        _SPRITEBITUNROLL(bm, (~bm));
                    }

                    //Only need to set the last byte if we're drawing in it of course
                    arduboy.sBuffer[bofs] = texByte;
                }

                #else // No loop unrolling

                uint8_t y = drawStartY;

                //Funny hack; code is written for loop unrolling first, so we have to kind of "fit in" to the macro system
                if((drawStartY & 7) == 0) thisWallByte--;

                do
                {
                    uint8_t bidx = y & 7;

                    // Every new byte, save the current (previous) byte and load the new byte from the screen. 
                    // This might be wasteful, as only the first and last byte technically need to pull from the screen. 
                    if(bidx == 0) {
                        _SPRITEWRITESCRNEXT();
                        _SPRITEREADSCRBYTE();
                    }

                    uint8_t bm = fastlshift8(bidx);
                    _SPRITEBITUNROLL(bm, ~bm);
                }
                while(++y < drawEndY); //EXCLUSIVE

                //The above loop specifically CAN'T reach the last byte, so although it's wasteful in the case of a 
                //sprite ending at the bottom of the screen, it's still better than always incurring an if statement... maybe.
                arduboy.sBuffer[bofs] = texByte;

                #endif

            }

            //This ONE step is why there has to be a big if statement up there. 
            texX += stepX;
        }
        while(++x < drawEndX); //EXCLUSIVE
        // ------- END CRITICAL SECTION -------------

        #ifdef PRINTSPRITEDATA
        //Clear a section for us to use
        constexpr uint8_t sdh = 10;
        arduboy.fillRect(0, HEIGHT - sdh, VIEWWIDTH, sdh, BLACK);
        //Print some junk
        tinyfont.setCursor(0, HEIGHT - sdh);
        tinyfont.print((float)transformXT, 4);
        tinyfont.print(" X");
        tinyfont.print(ssX);
        tinyfont.setCursor(0, HEIGHT - sdh + 5);
        tinyfont.print((float)transformYT, 4);
        tinyfont.print(" W");
        tinyfont.print(spriteWidth);
        #endif

    }
}

// Perform ONLY player movement updates! No drawing!
void movement()
{
    uflot newPosX = player.posX;
    uflot newPosY = player.posY;

    // move forward if no wall in front of you
    if (arduboy.pressed(UP_BUTTON))
    {
        newPosX += player.dirX * MOVESPEED;
        newPosY += player.dirY * MOVESPEED;
    }
    if (arduboy.pressed(DOWN_BUTTON))
    {
        newPosX -= player.dirX * MOVESPEED;
        newPosY -= player.dirY * MOVESPEED;
    }

    if(isCellSolid(&worldMap, newPosX.getInteger(), player.posY.getInteger())) newPosX = player.posX;
    if(isCellSolid(&worldMap, player.posX.getInteger(), newPosY.getInteger())) newPosY = player.posY;

    for(uint8_t i = 0; i < NUMBOUNDS; i++)
    {
        if(!(bounds[i].state & RSSTATEACTIVE))
            continue;

        if(newPosX > bounds[i].x1 && newPosX < bounds[i].x2 && player.posY > bounds[i].y1 && player.posY < bounds[i].y2)
            newPosX = player.posX;
        if(player.posX > bounds[i].x1 && player.posX < bounds[i].x2 && newPosY > bounds[i].y1 && newPosY < bounds[i].y2)
            newPosY = player.posY;
    }

    player.posX = newPosX;
    player.posY = newPosY;

    float rotation = 0;

    if (arduboy.pressed(RIGHT_BUTTON))
        rotation = -ROTSPEED;
    if (arduboy.pressed(LEFT_BUTTON))
        rotation = ROTSPEED;

    if(rotation)
    {
        float oldDirX = player.dirX;
        player.dirX = player.dirX * cos(rotation) - player.dirY * sin(rotation);
        player.dirY = oldDirX * sin(rotation) + player.dirY * cos(rotation);
    }
}

void runSprites()
{
    for(uint8_t i = 0; i < NUMSPRITES; i++)
    {
        if(!(sprites[i].state & RSSTATEACTIVE))
            continue;
        
        if(sprites[i].behavior)
            sprites[i].behavior(&sprites[i], &arduboy);
    }
}

inline bool inExit() { return getMapCell(&worldMap, (int)player.posX, (int)player.posY) == TILEEXIT; }

//Menu functionality, move the cursor, select things (redraws automatically)
void doMenu()
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
}

// Draw just the menu section, does not overwrite the raycast area
void drawMenu(bool showHint)
{
    constexpr uint8_t MENUX = 105;
    constexpr uint8_t MENUY = 22;
    constexpr uint8_t MENUSPACING = 6;

    fastClear(&arduboy, VIEWWIDTH, 0, WIDTH,HEIGHT);
    FASTRECT(arduboy, VIEWWIDTH + 1, 0, WIDTH - 1, HEIGHT - 1, WHITE);
    arduboy.drawPixel(VIEWWIDTH + 3, 2, WHITE);
    arduboy.drawPixel(WIDTH - 3, 2, WHITE);
    arduboy.drawPixel(VIEWWIDTH + 3, HEIGHT - 3, WHITE);
    arduboy.drawPixel(WIDTH - 3, HEIGHT - 3, WHITE);

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
}

void resetSprites()
{
    memset(sprites, 0, sizeof(RSprite) * NUMSPRITES);
}

uint8_t addSprite(float x, float y, uint8_t frame, uint8_t shrinkLevel, int8_t heightAdjust, behavior_func func)
{
    for(uint8_t i = 0; i < NUMSPRITES; i++)
    {
        if((sprites[i].state & 1) == 0)
        {
            sprites[i].x = muflot(x);
            sprites[i].y = muflot(y);
            sprites[i].frame = frame;
            sprites[i].state = 1 | ((shrinkLevel << 1) & RSSTATESHRINK) | (heightAdjust < 0 ? 16 : 0) | ((abs(heightAdjust) << 3) & RSTATEYOFFSET);
            sprites[i].behavior = func;
            return i;
            //return &sprites[i];
        }
    }

    return NULL;
}

// Generate a new maze and reset the game to an initial playable state
void generateMaze()
{
    clearRaycast(&arduboy);
    resetSprites();
    tinyfont.setCursor(12, 28);
    tinyfont.print(F("Generating maze"));
    arduboy.display();

    MazeSize mzs = getMazeSize(MAZESIZES, mazeSize);
    MazeType mzt = getMazeType(MAZETYPES, mazeType); 

    //Call the generator function chosen by the menu
    mzt.func(&worldMap, mzs.width, mzs.height, &player.posX, &player.posY, &player.dirX, &player.dirY);
    curWidth = mzs.width;
    curHeight = mzs.height;

    #ifdef ADDDEBUGAREA
    setMapCell(&worldMap, 5, 0, TILEDOOR);
    addSprite(4.5, 1.4, SPRITEBARREL, 1, 8, NULL);
    addSprite(6.5, 1.4, SPRITEBARREL, 1, 8, NULL);
    addSprite(7, 5, SPRITECHEST, 1, 8, NULL);
    addSprite(4, 3, SPRITEMONSTER, 1, 0, behavior_bat);
    uint8_t sp = addSprite(6, 3, SPRITELEVER, 1, 8, behavior_animate_16);
    sprites[sp].intstate[0] = SPRITELEVER;
    sprites[sp].intstate[1] = 2;
    for(uint8_t y = 1; y < 8; y++)
        for(uint8_t x = 3; x < 8; x++)
            setMapCell(&worldMap, x, y, TILEEMPTY);
    #endif
}


void setup()
{
    // Initialize the Arduboy
    arduboy.boot();
    arduboy.flashlight();
    arduboy.initRandomSeed();
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
        clearRaycast(&arduboy);

        constexpr uint8_t WINX = 22;
        constexpr uint8_t WINY = 32;

        tinyfont.setCursor(WINX, 24);
        tinyfont.print(F("COMPLETE!"));
        tinyfont.setCursor(WINX + 8, WINY);
    }
    else
    {
        #ifdef DRAWFOUNDATION
        raycastFoundation();
        #else
        clearRaycast();
        #endif

        raycastWalls(&player, &worldMap, &arduboy, tilesheet);
        #ifndef NOSPRITES
        runSprites();
        drawSprites();
        #endif
        movement();

        #ifdef DRAWMAPDEBUG
        drawMaze(&arduboy, worldMap, 0, 0);
        #endif
    }

    arduboy.display();
}
