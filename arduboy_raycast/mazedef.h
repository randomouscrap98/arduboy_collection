#pragma once

#include "utils.h"

constexpr uint8_t MAXMAPWIDTH = 24;
constexpr uint8_t MAXMAPHEIGHT = 24;

struct MazeSize 
{
    char name[4];
    uint8_t width;
    uint8_t height;
};

constexpr uint8_t MAZESIZECOUNT = 5;

constexpr MazeSize MAZESIZES[MAZESIZECOUNT] PROGMEM = {
    { "SML", 12, 12 },  //NOTE: must always be a multiple of 4 because each byte can hold 4 cells
    { "MED", 24, 24 },
    { "LRG", 36, 36 },
    { "XL ", 48, 48 },
    // { "XXL", 60, 60 } //Only if we have room
};

MazeSize getMazeSize(uint8_t index) 
{
    //A macro to generate the same old code, don't feel like making some generics madness
    getProgmemStruct(MazeSize, MAZESIZES, index)
}

