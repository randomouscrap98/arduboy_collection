#pragma once

#include "utils.h"
#include <FixedPoints.h>

// These are bitmasks to get data out of state
constexpr uint8_t RSSTATEACTIVE = 0b00000001;
constexpr uint8_t RSSTATESHRINK = 0b00000110;
constexpr uint8_t RSTATEYOFFSET = 0b11111000;

// Try to make this fit into as little space as possible
struct RSprite {
    muflot x; //Precision for x/y is low but doesn't really need to be high
    muflot y;
    uint8_t frame = 0;
    uint8_t state = 0; // First bit is active, next 2 are how many times to /2 for size

    //Note: if we sacrifice 2 bytes per sprite to store the distance (so 32 bytes currently), 
    //you can potentially save computation by sorting the sprite list itself. Sorted lists 
    //generally don't require as much computation. I don't know realistically how much is 
    //being saved though, so I'm not using that. I think having the sort struct below is
    //significantly more flexible and may actually end up being faster overall
};

// Sorted sprite. Needed to save memory in RSprite (distance). May add more data here
// for precomputation
struct SSprite {
    RSprite * sprite; 
    SFixed<11,4> dpx;   //Some precalc stuff to save compute (4 X NUMSPRITES extra bytes, here it's 128)
    SFixed<11,4> dpy;
    SFixed<11,4> distance;    //Unfortunately, distance kinda has to be large... 12 bits = 4096, should be more than enough
};
