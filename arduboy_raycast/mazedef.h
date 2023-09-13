#pragma once

#include "utils.h"

constexpr uint8_t MAXMAPWIDTH = 48;
constexpr uint8_t MAXMAPHEIGHT = 48;
constexpr uint8_t TILESPERBYTE = 4;
constexpr uint8_t REALMAPWIDTH = MAXMAPWIDTH / TILESPERBYTE;

constexpr uint8_t TILEWALL = 0b11; //Must always be full fill
constexpr uint8_t TILEEXIT = 0b10;
constexpr uint8_t TILEEMPTY = 0b00;

// Make sure to modify the constants here if you change the tile count

//Amount to shift value to get desired 2 bit x value in correct position
#define mazeXShift(x) ((x & TILEWALL) << 1)
//Get single index into 1d maze using 2d coordinates
#define mazeIndex(x, y) (y * REALMAPWIDTH + (x >> 2))


struct MazeSize 
{
    char name[4];
    uint8_t width;
    uint8_t height;
};

constexpr uint8_t MAZESIZECOUNT = 4;
constexpr MazeSize MAZESIZES[MAZESIZECOUNT] PROGMEM = {
    { "SML", 12, 12 },  //NOTE: must always be a multiple of 4 because each byte can hold 4 cells
    { "MED", 24, 24 },
    { "LRG", 36, 36 },
    { "XL ", 48, 48 },
    // { "XXL", 60, 60 } //Only if we have room
};

// For a given mazeSize index, get a copy (8 bytes-ish?) of the size description struct
MazeSize getMazeSize(uint8_t index) 
{
    //A macro to generate the same old code, don't feel like making some generics madness
    getProgmemStruct(MazeSize, MAZESIZES, index)
}

// Fill maze with all walls again (all of it)
void resetMaze(uint8_t * maze)
{
    memset(maze, 0xFF, (size_t)MAXMAPHEIGHT / TILESPERBYTE * MAXMAPWIDTH);
}


void setMazeCell(uint8_t * maze, uint8_t x, uint8_t y, uint8_t tile)
{
    //Go to the right location. Note that 2 would have to be changed
    maze = maze + mazeIndex(x, y);
    uint8_t xshift = mazeXShift(x);
    maze[0] = (maze[0] & ~(TILEWALL << xshift)) | (tile << xshift);
}

// This is called in a tight loop, I really want it inlined, I don't care about
// code size in this application
inline uint8_t getMazeCell(uint8_t * maze, uint8_t x, uint8_t y)
{
    return (*(maze + mazeIndex(x, y)) >> mazeXShift(x)) & TILEWALL;
}

uint8_t isCellSolid(uint8_t * maze, uint8_t x, uint8_t y)
{
    //Might change later, idk. Right now first bit determines solidity.
    return getMazeCell(maze, x, y) & 1;
}