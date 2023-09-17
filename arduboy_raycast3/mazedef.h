#pragma once

#include "utils.h"
#include "tile.h"

//Weird numbers but we always expect the outer walls to be filled + maze needs odd width.
//This way, the inner usable area is 16 or less, which takes up little memory + can be indexed
//with only 4 bits (if you offset by 1)
constexpr uint8_t MAXMAPWIDTH = 17;
constexpr uint8_t MAXMAPHEIGHT = 17;

constexpr uint8_t TILEEMPTY = 0x00;
constexpr uint8_t TILEWALL = 0x01;
constexpr uint8_t TILEEXIT = 0x02;


// Make sure to modify the constants here if you change the tile count

//Get single index into 1d maze using 2d coordinates
#define mazeIndex(x, y) ((y) * MAXMAPWIDTH + (x))
//Does what it says; it's a macro because I don't trust the compiler
#define isCellSolid(maze, x, y) (getMazeCell(maze, x, y) & 1)


void setMazeCell(uint8_t * maze, uint8_t x, uint8_t y, uint8_t tile)
{
    maze[mazeIndex(x,y)] = tile;
}

inline uint8_t getMazeCell(uint8_t * maze, uint8_t x, uint8_t y)
{
    return maze[mazeIndex(x,y)];
}

// Fill maze with all walls again (all of it)
void resetMaze(uint8_t * maze)
{
    memset(maze, TILEWALL, (size_t)MAXMAPHEIGHT * MAXMAPWIDTH);
}

// Draw the given maze starting at the given screen x + y
void drawMaze(Arduboy2Base * arduboy, uint8_t * map, uint8_t x, uint8_t y)
{
    //This is INCREDIBLY slow but should be fine
    for(uint8_t i = 0; i < MAXMAPHEIGHT; ++i)
        for(uint8_t j = 0; j < MAXMAPWIDTH; ++j)
            arduboy->drawPixel(x + j, y + i, getMazeCell(map, j, MAXMAPHEIGHT - i - 1) ? WHITE : BLACK);
}


struct MazeSize 
{
    char name[4];
    uint8_t width;
    uint8_t height;
};

constexpr uint8_t MAZESIZECOUNT = 2;
constexpr MazeSize MAZESIZES[MAZESIZECOUNT] PROGMEM = {
    { "SML", 11, 11 },  //NOTE: must always be 2N + 1
    { "MED", 17, 17 },
    //{ "LRG", 15, 15},
    //{ "XL ", 47, 47 },
    // { "XXL", 60, 60 } //Only if we have room (we don't)
};

// For a given mazeSize index, get a copy (8 bytes-ish?) of the size description struct
MazeSize getMazeSize(uint8_t index) 
{
    //A macro to generate the same old code, don't feel like making some generics madness
    getProgmemStruct(MazeSize, MAZESIZES, index)
}
