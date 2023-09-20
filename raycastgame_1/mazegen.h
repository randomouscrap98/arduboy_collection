#pragma once

#include "utils.h"
#include "rcmap.h"
#include "tilesheet.h"
//#include "dualsheet.h"

constexpr uint8_t ELLERHZCHANCE = 2; //This is actually 1 / 2 chance
constexpr uint8_t ELLERVTCHANCE = 2;
constexpr uint8_t ELLERROWSIZE = 32 >> 1; //This eller function will only operate on maps with dims 32 or smaller

constexpr uint8_t SPARSEFILLRATE = 8; //Actually 1 / N

//constexpr uint8_t ROOMSMAXDEPTH = 20;       // Each room element is 4 bytes FYI (Rect)
//constexpr uint8_t ROOMSMINWIDTH = 2;        // Each room's minimum dimension must be this.
//constexpr uint8_t ROOMSDOORBUFFER = 1;      // Doors should not be generated this close to the edge
//constexpr uint8_t ROOMSMAXWALLRETRIES = 3;  // This changes the chance of "big" rooms
//constexpr uint8_t ROOMSNINEWEIGHT = 9;      // Actually inverted
//constexpr uint8_t ROOMSCUBICLECHANCE = 5;   // Actually inverted
//constexpr uint8_t ROOMSCUBICLEMAXLENGTH = 4;


// Using some algorithm called "Eller's algorithm", which is constant memory. Note that width and height 
// are generated width and height; we don't use the width/height provided by map
void genMazeType(RcMap * map, uint8_t width, uint8_t height, uflot * posX, uflot * posY, float * dirX, float * dirY)
{
    fillMap(map, TILEWALL);

    //They MUST be odd
    oddify(width);
    oddify(height);

    // Also need to know which set each cell is in (basic row)
    uint16_t row[ELLERROWSIZE]; 
    uint16_t setId = 0;

    uint8_t xStart = 1;
    uint8_t yStart = 1;
    uint8_t xEnd = width - 2;
    uint8_t yEnd = height - 2;

    //Main algorithm loop (once per row)
    for(uint8_t y = yStart; y <= yEnd; y += 2)
    {
        // Need to initialize every other block to be a "cell". Also initialize the 
        // row set tracker
        for(uint8_t x = xStart; x <= xEnd; x += 2)
        {
            setMapCell(map, x, y, TILEEMPTY);

            // This works no matter which row we're on because the top row has a 
            // full ceiling, and thus will "initialize" the row sets to all individual
            // sets. Otherwise, the row retains the set from the previous row.
            if(getMapCell(map, x, y - 1) == TILEWALL)
                row[x >> 1] = ++setId;
        }
        
        // Now iterate over cells again. For each one, we simply, with some probability, 
        // join our cell with the next. We actually look backwards to make things easier
        for(uint8_t x = xStart + 2; x <= xEnd; x += 2)
        {
            uint8_t xrow = x >> 1;
            // Tear down the wall between them if the sets don't match and either a random
            // chance OR we're on the last row
            if(row[xrow - 1] != row[xrow] && (random(ELLERHZCHANCE) == 0 || y == yEnd)) {
                // Merge set by flood filling essentially
                uint8_t ovr = row[xrow];
                uint8_t flood = row[xrow - 1];
                for(uint8_t i = xrow; i < ELLERROWSIZE; i++)
                {
                    if(row[i] != ovr)
                        break;
                    row[i] = flood; //min(row[xrow - 1], row[xrow]);
                }
                setMapCell(map, x - 1, y, TILEEMPTY);
            }
        }

        // Iterate over the cells one last time, but only if this isn't the last row. 
        if(y != yEnd)
        {
            for (uint8_t x = xStart; x <= xEnd; x += 2)
            {
                // Randomly connect with the next row down, but it's a bit compute intensive
                // because we must continuously scan the rest of the row to see if this 
                // is the last instance of this set AND there are no other sets, and if so, 
                // 100% must connect. (because memory bound)
                uint8_t isLast = 1;
                for(uint8_t i = x + 2; i <= xEnd; i += 2)
                {
                    if(row[(x >> 1)] == row[(i >> 1)])
                    {
                        isLast = 0;
                        break;
                    }
                }

                uint8_t mustConnect = isLast;

                if(isLast)
                {
                    //Full scan whole row to see if any cells in our set
                    //connect to the next row
                    for(uint8_t i = xStart; i <= xEnd; i += 2)
                    {
                        if(row[(x >> 1)] == row[(i >> 1)] && getMapCell(map, i, y + 1) == TILEEMPTY)
                        {
                            mustConnect = 0;
                            break;
                        }
                    }
                }

                if(mustConnect || random(ELLERVTCHANCE) == 0)
                {
                    // DON'T need to set the row, we scan for walls in the next iteration
                    setMapCell(map, x, y + 1, TILEEMPTY);
                }
            }
        }
    } 

    setMapCell(map, xEnd + 1, yEnd, TILEEXIT);

    * posX = 1.6; *posY = 1.6;

    if(getMapCell(map, xStart + 1, yStart) == TILEEMPTY) {
        * dirX = 1;
        * dirY = 0;
    }
    else {
        * dirY = 1;
        * dirX = 0;
    }
}

void genSparseRandom(RcMap * map, uint8_t width, uint8_t height, uflot * posX, uflot * posY, float * dirX, float * dirY)
{
    fillMap(map, TILEWALL);

    uint8_t xStart = 1;
    uint8_t yStart = 1;
    uint8_t xEnd = width - 2;
    uint8_t yEnd = height - 2;

    for(uint8_t y = yStart; y <= yEnd; ++y)
        for(uint8_t x = xStart; x <= xEnd; ++x)
            setMapCell(map, x, y, TILEEMPTY);

    //Generate an amount of random dots proportional to filling 1/8 of the total inner area. Don't place any
    //on the edges
    uint8_t inWidth = width - 4;
    uint8_t inHeight = height - 4;
    for(uint8_t i = 0; i < inWidth * inHeight / SPARSEFILLRATE; ++i)
    {
        setMapCell(map, xStart + 1 + random(inWidth), yStart + 1 + random(inHeight), TILEWALL);
    }

    setMapCell(map, xEnd + 1, yEnd, TILEEXIT);

    * posX = 1.6; *posY = 1.6;
    * dirX = 1;
    * dirY = 0;
}

//struct RoomStack {
//    MRect rooms[ROOMSMAXDEPTH];
//    uint8_t count = 0;  // 1 past the "top" of the stack
//};
//
//bool pushRoom(RoomStack * stack, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
//{
//    if(stack->count == ROOMSMAXDEPTH)
//        return false;
//    MRect * r = &stack->rooms[stack->count];
//    r->x = x;
//    r->y = y;
//    r->w = w;
//    r->h = h;
//    stack->count += 1;
//    return true;
//}
//
//MRect popRoom(RoomStack * stack)
//{
//    MRect result;
//
//    if(stack->count != 0)
//    {
//        stack->count -= 1;
//        memcpy(&result, &stack->rooms[stack->count], sizeof(MRect));
//    }
//
//    return result;
//}
//
//// Split rooms into smaller rooms randomly until a minimum is reached. If a room cannot be split but it's of 
//// certain dimensions, randomly add "interesting" features to it.
//void genRoomsType(uint8_t * map, uint8_t width, uint8_t height, uflot * posX, uflot * posY, float * dirX, float * dirY)
//{
//    //Note: For rooms type, doesn't have to be odd
//    resetMaze(map);
//
//    //Now clear out the whole inside. Might as well make the rect to represent it.
//    MRect crect; 
//    crect.x = 1; 
//    crect.y = 1; 
//    crect.w = width - 2; 
//    crect.h = height -2;
//
//    for(uint8_t i = 0; i < crect.h; ++i)
//        for(uint8_t j = 0; j < crect.w; ++j)
//            setMazeCell(map, crect.x + j, crect.y + i, TILEEMPTY);
//
//    RoomStack stack;
//
//    //Push the main room onto the stack
//    pushRoom(&stack, crect.x, crect.y, crect.w, crect.h);
//
//    //Now the main loop
//    while(stack.count)
//    {
//        crect = popRoom(&stack); //this is 2 memcpys and a function call but it doesn't matter, it's just the generator
//
//        //Figure out the length of the longer side and thus the amount of places we can put a wall
//        uint8_t longest = max(crect.w, crect.h);
//        uint8_t shortest = min(crect.w, crect.h);
//        int8_t wallSpace = longest - 2 * ROOMSMINWIDTH;
//        int8_t doorSpace = shortest - 2 * ROOMSDOORBUFFER;
//        uint8_t wdiv = 0;
//
//        //Only partition the room if there's wallSpace and doorSpace. We sometimes also don't touch
//        //rooms that are exactly 9 wide.
//        if(wallSpace > 0 && doorSpace > 0 && !(crect.w == 9 && random(ROOMSNINEWEIGHT) == 0))
//        {
//            //We can precalc the door position. Remember, the walls are all 'shorter' since they break up 'longer'
//            uint8_t door = ROOMSDOORBUFFER + random(doorSpace);
//            uint8_t exact;
//
//            for(uint8_t retries = 0; retries < ROOMSMAXWALLRETRIES; retries++)
//            {
//                uint8_t rnd = ROOMSMINWIDTH + random(wallSpace);
//                if(longest == crect.w)
//                {
//                    //X and Y are always INSIDE the room, not in the walls
//                    exact = crect.x + rnd;
//                    if(getMazeCell(map, exact, crect.y - 1) == TILEWALL &&
//                        getMazeCell(map, exact, crect.y + crect.h) == TILEWALL)
//                    {
//                        wdiv = exact;
//                        // Now draw the wall, add a door, and add the two sides to the stack
//                        for(uint8_t i = 0; i < shortest; i++)
//                            if(i != door)
//                                setMazeCell(map, exact, crect.y + i, TILEWALL);
//                        //Two sides are the original x,y,h + smaller width, then wall+1x,y,h + smaller width
//                        pushRoom(&stack, crect.x, crect.y, exact - crect.x, crect.h);
//                        pushRoom(&stack, exact + 1, crect.y, crect.w - (exact - crect.x) - 1, crect.h);
//                        break;
//                    }
//                }
//                else
//                {
//                    exact = crect.y + rnd;
//                    if(getMazeCell(map, crect.x - 1, exact) == TILEWALL &&
//                        getMazeCell(map, crect.x + crect.w, exact) == TILEWALL)
//                    {
//                        wdiv = exact;
//                        // Now draw the wall, add a door, and add the two sides to the stack
//                        for(uint8_t i = 0; i < shortest; i++)
//                            if(i != door)
//                                setMazeCell(map, crect.x + i, exact, TILEWALL);
//                        //Two sides are original x,y,w,smaller h, then x,wall+1,w, smaller h
//                        pushRoom(&stack, crect.x, crect.y, crect.w, exact - crect.y);
//                        pushRoom(&stack, crect.x, exact + 1, crect.w, crect.h - (exact - crect.y) - 1);
//                        break;
//                    }
//                }
//            }
//        }
//
//        // Yes, I am AWARE this is not a scoped macro, I'm putting it here so >> I << 
//        // am aware it is supposed to be scoped. It's too hard to get these out of RAM otherwise.
//        // The compiler refused to put any of my arrays into PROGMEM even though I asked, so IDK
//        // what's going on there.
//        #define df(ofsx, ofsy) setMazeCell(map, crect.x + ofsx, crect.y + ofsy, TILEWALL)
//
//        //If the room was not divided, generate some things
//        if(wdiv == 0)
//        {
//            // 9 and 7 are EXCEPTIONALLY rare, so they're kind of fun to stumble across.
//            // The first checks are all "exact match" rooms. If not, go into the 
//            // "fuzzy match" rooms.
//            if(crect.w == 9 && crect.h >= 12) {
//                for(uint8_t i = 0; i < 5; ++i) {
//                    setMazeCell(map, crect.x + i + 2, crect.y + crect.h - 3, TILEWALL);
//                    setMazeCell(map, crect.x + i + 2, crect.y + crect.h - 7, TILEWALL);
//                    setMazeCell(map, crect.x + 2, crect.y + crect.h - i - 3, TILEWALL);
//                    setMazeCell(map, crect.x + 6, crect.y + crect.h - i - 3, TILEWALL);
//                }
//                setMazeCell(map, crect.x + 4, crect.y + crect.h - 7, TILEEMPTY);
//                setMazeCell(map, crect.x + 4, crect.y + crect.h - 5, TILEEXIT);
//                for(int8_t y = crect.y + crect.h - 10; y >= crect.y + 2; y -= 3) {
//                    setMazeCell(map, crect.x + 2, y, TILEWALL);
//                    setMazeCell(map, crect.x + 6, y, TILEWALL);
//                }
//            }
//            else if(crect.w == 9 && crect.h == 9) {
//                df(2,3); df(2,2); df(3,2);
//                df(5,2); df(6,2); df(6,3);
//                df(6,5); df(6,6); df(5,6);
//                df(3,6); df(2,6); df(2,5);
//            }
//            else if(crect.w == 7 && crect.h == 7) {
//                df(2,2); df(2,4);
//                df(4,2); df(4,4);
//            }
//            else if(crect.w == 6 && crect.h == 6) {
//                df(1,1); df(4,1);
//                df(1,4); df(4,4);
//            }
//            else if(crect.w == 5 && crect.h == 5) {
//                df(2,2);
//            }
//            else if(crect.w == 3) {
//                // We can do a bit of fuzzy stuff here, it's fine. This is all 
//                // imprecise anyway; 3 wide rooms are common
//                if(crect.h > 5 && (crect.h & 1)) {
//                    for(uint8_t i = crect.y + 2; i < crect.y + crect.h - 2; i += 2)
//                        setMazeCell(map, crect.x + 1, i, TILEWALL);
//                }
//                else {
//                    if(getMazeCell(map, crect.x + 1, crect.y - 1) == TILEWALL && random(5) == 0)
//                        setMazeCell(map, crect.x + 1, crect.y, TILEWALL);
//                }
//            }
//            else {
//                // These don't have at least one required exact width or height, so they're "fuzzy"
//                if(crect.w > 7 && crect.h > ROOMSCUBICLEMAXLENGTH * 2)
//                {
//                    // Go around the perimeter and, with a low chance, add random length walls
//                    for(uint8_t x = 0; x < crect.w; ++x)
//                    {
//                        if(getMazeCell(map, crect.x + x, crect.y - 1) == TILEWALL && random(ROOMSCUBICLECHANCE) == 0)
//                            for(int8_t i = random(ROOMSCUBICLEMAXLENGTH); i >= 0; --i)
//                                setMazeCell(map, crect.x + x, crect.y + i, TILEWALL);
//                        if(getMazeCell(map, crect.x + x, crect.y + crect.h) == TILEWALL && random(ROOMSCUBICLECHANCE) == 0)
//                            for(int8_t i = random(ROOMSCUBICLEMAXLENGTH); i >= 0; --i)
//                                setMazeCell(map, crect.x + x, crect.y + crect.h - 1 - i, TILEWALL);
//                    }
//                }
//            }
//        }
//    }
//
//    setMazeCell(map, width - 1, height - 3, TILEEXIT);
//
//    * posX = 1.6; *posY = 1.6;
//
//    if(getMazeCell(map, 2, 1) == TILEEMPTY) {
//        * dirX = 1;
//        * dirY = 0;
//    }
//    else {
//        * dirY = 1;
//        * dirX = 0;
//    }
//}


// Unused for now; may come back to this
//void genCellType(uint8_t * map, uint8_t width, uint8_t height, float * posX, float * posY, float * dirX, float * dirY)
//{
//    resetMaze(map);
//
//    //They must be odd
//    oddify(width);
//    oddify(height);
//}

struct MazeSize 
{
    char name[4];
    uint8_t width;
    uint8_t height;
};

struct MazeType
{
    char name[4];
    void (*func)(RcMap*, uint8_t, uint8_t, uflot *, uflot *, float *, float *);
};

// For a given mazeSize index, get a copy (8 bytes-ish?) of the size description struct
MazeSize getMazeSize(const MazeSize * sizes, uint8_t index) 
{
    MazeSize result;
    memcpy_P(&result, &sizes[index], sizeof(MazeSize));
    return result;
}

MazeType getMazeType(const MazeType * types, uint8_t index) 
{
    MazeType result;
    memcpy_P(&result, &types[index], sizeof(MazeType));
    return result;
}
