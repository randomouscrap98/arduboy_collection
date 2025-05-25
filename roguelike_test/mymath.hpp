#pragma once

#include <Arduboy2.h>

constexpr uint8_t DIRNORTH = 0;
constexpr uint8_t DIREAST = 1;
constexpr uint8_t DIRSOUTH = 2;
constexpr uint8_t DIRWEST = 3;

void cardinal_to_dir(uint8_t cardinal, int8_t *dx, int8_t *dy);
float cardinal_to_rad(uint8_t cardinal);

struct prng_state {
  uint8_t a, b, c, d;
};

uint8_t prng();
void prng_seed(uint16_t seed);
prng_state prng_snapshot();
void prng_restore(prng_state state);

uint16_t lcg_shuffle(uint16_t lcg_seed, uint16_t iteration);

#define RANDB(x) (prng() % (x))

// A rectangle starting at x, y and having side w, h
struct MRect {
  uint8_t x, y, w, h;
};
