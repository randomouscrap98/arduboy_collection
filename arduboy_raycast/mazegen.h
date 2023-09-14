#pragma once

#include "utils.h"
#include "mazedef.h"

constexpr uint8_t ELLERHZCHANCE = 2; //This is actually 1 / 2 chance
constexpr uint8_t ELLERVTCHANCE = 2;
constexpr uint8_t ELLERROWSIZE = MAXMAPWIDTH >> 1;

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

void genRoomsType(uint8_t * map, uint8_t width, uint8_t height, float * posX, float * posY, float * dirX, float * dirY)
{
    resetMaze(map);

    //They must be odd
    oddify(width);
    oddify(height);
}

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

constexpr uint8_t MAZETYPECOUNT = 3;

constexpr MazeType MAZETYPES[MAZETYPECOUNT] PROGMEM = {
    { "MAZ", &genMazeType },
    { "RMS", &genRoomsType },
    { "CEL", &genCellType }
};

MazeType getMazeType(uint8_t index) 
{
    //A macro to generate the same old code, don't feel like making some generics madness
    getProgmemStruct(MazeType, MAZETYPES, index)
}