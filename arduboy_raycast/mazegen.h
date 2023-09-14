#pragma once

#include "utils.h"
#include "mazedef.h"

constexpr uint8_t ELLERHZCHANCE = 2; //This is actually 1 / 2
constexpr uint8_t ELLERVTCHANCE = 2;

// Using some algorithm called "Eller's algorithm", which is constant memory.
void genMazeType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);

    //They must be odd
    oddify(width);
    oddify(height);

    // Also need to know which set each cell is in (basic row)
    uint16_t row[MAXMAPWIDTH >> 1]; 
    uint16_t setId = 0;

    uint8_t cellCount = width >> 1;

    // Initialize first row to have each cell be in a different set

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
        // 
    }

    setMazeCell(map, width - 1, 1, TILEEXIT);
}

void genRoomsType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);

    //They must be odd
    oddify(width);
    oddify(height);
}

void genCellType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);

    //They must be odd
    oddify(width);
    oddify(height);
}


struct MazeType
{
    char name[4];
    void (*func)(uint8_t*, uint8_t, uint8_t);
    //void * func;
};

constexpr uint8_t MAZETYPECOUNT = 3;

constexpr MazeType MAZETYPES[MAZETYPECOUNT] = {
    { "MAZ", &genMazeType },
    { "RMS", &genRoomsType },
    { "CEL", &genCellType }
};
