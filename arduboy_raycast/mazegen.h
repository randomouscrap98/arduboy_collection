#pragma once

#include "utils.h"
#include "mazedef.h"

constexpr uint8_t ELLERHZCHANCE = 2; //This is actually 1 / 2 chance
constexpr uint8_t ELLERVTCHANCE = 2;
constexpr uint8_t ELLERROWSIZE = MAXMAPWIDTH >> 1;

constexpr uint8_t ROOMSMAXDEPTH = 20;    // Each room element is 4 bytes FYI (Rect)
constexpr uint8_t ROOMSMINWIDTH = 2;    // Each room's minimum dimension must be this.
constexpr uint8_t ROOMSDOORBUFFER = 1;  // Doors should not be generated this close to the edge
constexpr uint8_t ROOMSMAXWALLRETRIES = 3; // This changes the chance of "big" rooms
//constexpr uint8_t ROOMSBIGMAX = 10;     // The maximum size of a "big" room
//constexpr uint8_t ROOMSBIGCHANCE = 3;   // Actually inverted

// Using some algorithm called "Eller's algorithm", which is constant memory.
void genMazeType(uint8_t * map, uint8_t width, uint8_t height, float * posX, float * posY, float * dirX, float * dirY)
{
    resetMaze(map);

    //They must be odd
    oddify(width);
    oddify(height);

    // Also need to know which set each cell is in (basic row)
    uint16_t row[ELLERROWSIZE]; 
    uint16_t setId = 0;

    uint8_t yEnd = height - 2;

    //Main algorithm loop (once per row)
    for(uint8_t y = 1; y < height; y += 2)
    {
        // Need to initialize every other block to be a "cell". Also initialize the 
        // row set tracker
        for(uint8_t x = 1; x < width; x += 2)
        {
            setMazeCell(map, x, y, 0);

            // This works no matter which row we're on because the top row has a 
            // full ceiling, and thus will "initialize" the row sets to all individual
            // sets. Otherwise, the row retains the set from the previous row.
            if(getMazeCell(map, x, y - 1) == TILEWALL)
                row[x >> 1] = ++setId;
        }
        
        // Now iterate over cells again. For each one, we simply, with some probability, 
        // join our cell with the next. We actually look backwards to make things easier
        for(uint8_t x = 3; x < width; x += 2)
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
                setMazeCell(map, x - 1, y, TILEEMPTY);
            }
        }

        // Iterate over the cells one last time, but only if this isn't the last row. 
        if(y != yEnd)
        {
            for (uint8_t x = 1; x < width; x += 2)
            {
                // Randomly connect with the next row down, but it's a bit compute intensive
                // because we must continuously scan the rest of the row to see if this 
                // is the last instance of this set AND there are no other sets, and if so, 
                // 100% must connect. (because memory bound)
                uint8_t isLast = 1;
                for(uint8_t i = x + 2; i < width; i += 2)
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
                    for(uint8_t i = 1; i < width; i += 2)
                    {
                        if(row[(x >> 1)] == row[(i >> 1)] && getMazeCell(map, i, y + 1) == TILEEMPTY)
                        {
                            mustConnect = 0;
                            break;
                        }
                    }
                }

                if(mustConnect || random(ELLERVTCHANCE) == 0)
                {
                    // DON'T need to set the row, we scan for walls in the next iteration
                    setMazeCell(map, x, y + 1, TILEEMPTY);
                }
            }
        }
    } 

    setMazeCell(map, width - 1, yEnd, TILEEXIT);

    * posX = 1.6;
     *posY = 1.6;

    if(getMazeCell(map, 2, 1) == TILEEMPTY)
    {
        * dirX = 1;
        * dirY = 0;
    }
    else
    {
        * dirY = 1;
        * dirX = 0;
    }
}


struct RoomStack {
    MRect rooms[ROOMSMAXDEPTH];
    uint8_t count = 0;  // 1 past the "top" of the stack
};

bool pushRoom(RoomStack * stack, uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    if(stack->count == ROOMSMAXDEPTH)
        return false;
    MRect * r = &stack->rooms[stack->count];
    r->x = x;
    r->y = y;
    r->w = w;
    r->h = h;
    stack->count += 1;
    return true;
}

MRect popRoom(RoomStack * stack)
{
    MRect result;

    if(stack->count != 0)
    {
        stack->count -= 1;
        memcpy(&result, &stack->rooms[stack->count], sizeof(MRect));
    }

    return result;
}

// Split rooms into smaller rooms randomly until a minimum is reached. If a room cannot be split but it's of 
// certain dimensions, randomly add "interesting" features to it.
void genRoomsType(uint8_t * map, uint8_t width, uint8_t height, float * posX, float * posY, float * dirX, float * dirY)
{
    //Note: For rooms type, doesn't have to be odd
    resetMaze(map);

    //Now clear out the whole inside. Might as well make the rect to represent it.
    MRect crect; 
    crect.x = 1; 
    crect.y = 1; 
    crect.w = width - 2; 
    crect.h = height -2;

    for(uint8_t i = 0; i < crect.h; ++i)
        for(uint8_t j = 0; j < crect.w; ++j)
            setMazeCell(map, crect.x + j, crect.y + i, TILEEMPTY);

    RoomStack stack;

    //Push the main room onto the stack
    pushRoom(&stack, crect.x, crect.y, crect.w, crect.h);

    //Now the main loop
    while(stack.count)
    {
        crect = popRoom(&stack); //this is 2 memcpys and a function call but it doesn't matter, it's just the generator

        //Figure out the length of the longer side and thus the amount of places we can put a wall
        uint8_t longest = max(crect.w, crect.h);
        uint8_t shortest = min(crect.w, crect.h);
        int8_t wallSpace = longest - 2 * ROOMSMINWIDTH;
        int8_t doorSpace = shortest - 2 * ROOMSDOORBUFFER;
        uint8_t wdiv = 0;

        //Only partition the room if there's wallSpace.
        if(wallSpace > 0 && doorSpace > 0)
        {
            //We can precalc the door position. Remember, the walls are all 'shorter' since they break up 'longer'
            uint8_t door = ROOMSDOORBUFFER + random(doorSpace);
            uint8_t exact;

            for(uint8_t retries = 0; retries < ROOMSMAXWALLRETRIES; retries++)
            {
                uint8_t rnd = ROOMSMINWIDTH + random(wallSpace);
                if(longest == crect.w)
                {
                    //X and Y are always INSIDE the room, not in the walls
                    exact = crect.x + rnd;
                    if(getMazeCell(map, exact, crect.y - 1) == TILEWALL &&
                        getMazeCell(map, exact, crect.y + crect.h) == TILEWALL)
                    {
                        wdiv = exact;
                        // Now draw the wall, add a door, and add the two sides to the stack
                        for(uint8_t i = 0; i < shortest; i++)
                            if(i != door)
                                setMazeCell(map, exact, crect.y + i, TILEWALL);
                        //Two sides are the original x,y,h + smaller width, then wall+1x,y,h + smaller width
                        pushRoom(&stack, crect.x, crect.y, exact - crect.x, crect.h);
                        pushRoom(&stack, exact + 1, crect.y, crect.w - (exact - crect.x) - 1, crect.h);
                        break;
                    }
                }
                else
                {
                    exact = crect.y + rnd;
                    if(getMazeCell(map, crect.x - 1, exact) == TILEWALL &&
                        getMazeCell(map, crect.x + crect.w, exact) == TILEWALL)
                    {
                        wdiv = exact;
                        // Now draw the wall, add a door, and add the two sides to the stack
                        for(uint8_t i = 0; i < shortest; i++)
                            if(i != door)
                                setMazeCell(map, crect.x + i, exact, TILEWALL);
                        //Two sides are original x,y,w,smaller h, then x,wall+1,w, smaller h
                        pushRoom(&stack, crect.x, crect.y, crect.w, exact - crect.y);
                        pushRoom(&stack, crect.x, exact + 1, crect.w, crect.h - (exact - crect.y) - 1);
                        break;
                    }
                }
            }
        }

        //If the room was not divided, generate some things
        if(wdiv == 0)
        {

        }
    }

    setMazeCell(map, width - 1, height - 3, TILEEXIT);

    * posX = 1.6;
     *posY = 1.6;

    if(getMazeCell(map, 2, 1) == TILEEMPTY)
    {
        * dirX = 1;
        * dirY = 0;
    }
    else
    {
        * dirY = 1;
        * dirX = 0;
    }
}


// Unused for now; may come back to this
void genCellType(uint8_t * map, uint8_t width, uint8_t height, float * posX, float * posY, float * dirX, float * dirY)
{
    resetMaze(map);

    //They must be odd
    oddify(width);
    oddify(height);
}


struct MazeType
{
    char name[4];
    void (*func)(uint8_t*, uint8_t, uint8_t, float *, float *, float *, float *);
};

constexpr uint8_t MAZETYPECOUNT = 2;

constexpr MazeType MAZETYPES[MAZETYPECOUNT] PROGMEM = {
    { "MAZ", &genMazeType },
    { "BKR", &genRoomsType },
    //{ "CEL", &genCellType }
};

MazeType getMazeType(uint8_t index) 
{
    //A macro to generate the same old code, don't feel like making some generics madness
    getProgmemStruct(MazeType, MAZETYPES, index)
}