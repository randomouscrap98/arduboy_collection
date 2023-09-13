#pragma once

// Generates code for pulling a single struct of type t from a progmem
// data array of structs d at index idx
#define getProgmemStruct(t, d, idx) \
    t v; uint8_t * vs = (uint8_t *)&v, * s = (uint8_t *)&d[idx]; \
    for(uint8_t i = 0; i < sizeof(t); ++i) vs[i] = pgm_read_byte(s + i); \
    return v;

// Mod the given menu position m the given amount v against a max of x
#define menumod(m, v, x) m = (m + v + x) % x