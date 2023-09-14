#pragma once

#include "utils.h"
#include "mazedef.h"

void genMazeType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);

    for(uint8_t y = 1; y < height - 1; y += 2)
        for(uint8_t x = 1; x < width - 1; x += 1)
            setMazeCell(map, x, y, 0);

    for(uint8_t x = 1; x < width - 1; x += 2)
        for(uint8_t y = 1; y < height - 1; y += 1)
            setMazeCell(map, x, y, 0);
    
    setMazeCell(map, width - 1, 1, TILEEXIT);
}

void genRoomsType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);
}

void genCellType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);
}


// Draw the given maze starting at the given screen x + y
void drawMaze(Arduboy2Base * arduboy, uint8_t * map, uint8_t x, uint8_t y)
{
    //This is INCREDIBLY slow but should be fine
    for(uint8_t i = 0; i < MAXMAPHEIGHT; ++i)
        for(uint8_t j = 0; j < MAXMAPWIDTH; ++j)
            arduboy->drawPixel(x + j, y + i, getMazeCell(map, j, i) ? WHITE : BLACK);
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
