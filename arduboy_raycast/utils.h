#pragma once

// I modify these for testing, just nice to have it abstracted
typedef SFixed<7,8> flot;
typedef UFixed<8,8> uflot;

// Funny hack constants. We're working with very small ranges, so 
// the values have to be picked carefully. The following must remain true:
// 1 / NEARZEROFIXED < MAXFIXED. It may even need to be < MAXFIXED / 2
constexpr uflot MAXFIXED = 255;
constexpr uflot NEARZEROFIXED = 1.0f / 128; // Prefer accuracy (fixed decimal exact)

// Generates code for pulling a single struct of type t from a progmem
// data array of structs d at index idx
#define getProgmemStruct(t, d, idx) \
    t v; uint8_t * vs = (uint8_t *)&v, * s = (uint8_t *)&d[idx]; \
    for(uint8_t i = 0; i < sizeof(t); ++i) vs[i] = pgm_read_byte(s + i); \
    return v;

// Mod the given menu position m the given amount v against a max of x
#define menumod(m, v, x) m = (m + v + x) % x

// Make value odd by subtracting 1 if necessary
#define oddify(v) if((v & 1) == 0) v -= 1

// A rectangle starting at x, y and having side w, h
struct MRect {
    uint8_t x = 0;
    uint8_t y = 0;
    uint8_t w = 0;
    uint8_t h = 0;
};

// Clear screen in a fast block. Note that y will be shifted down and y2
// shifted up to the nearest multiple of 8 to be byte aligned, so you 
// may not get the exact box you want. X2 and Y2 are exclusive
void fastClear(Arduboy2Base * arduboy, uint8_t x, uint8_t y, uint8_t x2, uint8_t y2)
{
    uint8_t yEnd = (y2 >> 3) + (y2 & 7 ? 1 : 0);
    //Arduboy2 fillrect is absurdly slow; I have the luxury of doing this instead
    for(uint8_t i = y >> 3; i < yEnd; ++i)
        memset(arduboy->sBuffer + (i << 7) + x, 0, x2 - x);
}