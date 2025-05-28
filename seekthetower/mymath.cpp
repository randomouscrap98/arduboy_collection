#include "mymath.hpp"

void cardinal_to_dir(uint8_t cardinal, int8_t *dx, int8_t *dy) {
  *dx = 0;
  *dy = 0;
  switch (cardinal) {
  case 0: // North / up
    *dy = 1;
    return;
  case 1: // East / right
    *dx = 1;
    return;
  case 2: // South / down
    *dy = -1;
    return;
  case 3:
  default: // Default west? eehhhh
    *dx = -1;
    return;
  }
}

float cardinal_to_rad(uint8_t cardinal) {
  switch (cardinal) {
  case 0: // North / up
    return PI * 0.5f;
  case 1: // East / right
    return 0;
  case 2: // South / down
    return PI * 1.5f;
  case 3:
  default: // Default west? eehhhh
    return PI;
  }
}

static struct prng_state pstate = {.a = 0xfa, .b = 0xee, .c = 0xee, .d = 0xee};

#define rot8(x, k) (((x) << (k)) | ((x) >> (8 - (k))))
uint8_t prng() { // jsf8, see https://filterpaper.github.io/prng.html#jsf8
  uint8_t e = pstate.a - rot8(pstate.b, 1);
  pstate.a = pstate.b ^ rot8(pstate.c, 4);
  pstate.b = pstate.c + pstate.d;
  pstate.c = pstate.d + e;
  return pstate.d = e + pstate.a;
}

void prng_seed(uint16_t seed) {
  pstate.a = 0xf1; // From https://filterpaper.github.io/prng.html#jsf8
  pstate.b = 0xee;
  // Just a handful of seeds out of 65,536 are bad (luckily)
  if (seed == 6332 || seed == 18002 || seed == 19503)
    pstate.b = 0xef;
  pstate.c = (seed >> 8) & 0xff;
  pstate.d = seed & 0xff;
}

// Copies internal pstate (4 bytes only, luckily)
prng_state prng_snapshot() { return pstate; }
void prng_restore(prng_state state) { pstate = state; }

// Allows access to a shuffled list of 65,536 values in a unique order per
// "lcg_seed". Useful for generating a list of further seeds for use in dungeon
// generation for a given player
uint16_t lcg_shuffle(uint16_t lcg_seed, uint16_t iteration) {
  // Player seed doesn't change and sets up a linear congruential generator as a
  // shuffle (period 2^16)
  uint16_t a = (lcg_seed & 0xF000) | 0x0805;
  uint16_t c = 2 * (lcg_seed & 0xFFF) + 1;
  // Iteration is just the Nth value from lcg:
  return a * iteration + c;
}
