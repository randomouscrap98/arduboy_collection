#pragma once

#include "utils.h"
#include "mazedef.h"

struct MazeType
{
    char name[4];
    void (*func)(uint8_t*, uint8_t, uint8_t);
    //void * func;
};


void genMazeType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);

    for(uint8_t y = 1; y < height - 1; y += 2)
        for(uint8_t x = 1; x < width - 1; x += 1)
            setMazeCell(map, x, y, 0);

    for(uint8_t x = 1; x < width - 1; x += 2)
        for(uint8_t y = 1; y < height - 1; y += 1)
            setMazeCell(map, x, y, 0);
}

void genRoomsType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);
}

void genCellType(uint8_t * map, uint8_t width, uint8_t height)
{
    resetMaze(map);
}

constexpr uint8_t MAZETYPECOUNT = 3;

constexpr MazeType MAZETYPES[MAZETYPECOUNT] = {
    { "MAZ", &genMazeType },
    { "RMS", &genRoomsType },
    { "CEL", &genCellType }
};
