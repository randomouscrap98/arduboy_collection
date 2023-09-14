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