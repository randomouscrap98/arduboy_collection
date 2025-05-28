#include "graphics.hpp"
#include "mymath.hpp"

constexpr uint16_t TINYDIGITS[] PROGMEM = {
    0xF9F, 0xAF8, 0xDDA, 0x9DF, 0x74F, 0xBDD, 0xFDC, 0x11F, 0xFDF, 0x75F,
};

// Print tiny digits at given location
void print_tinydigit(uint8_t *buffer, uint8_t v, uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < 3; i++) {
    uint8_t dig = ((pgm_read_word(TINYDIGITS + v)) >> ((2 - i) * 4)) & 0xF;
    buffer[x + i + (y >> 3) * WIDTH] |= (dig << (y & 7));
  }
}

// Print a number to the given amount of digits
void print_tinynumber(uint8_t *buffer, uint16_t v, uint8_t w, uint8_t x,
                      uint8_t y) {
  for (uint8_t i = 0; i < w; i++) {
    print_tinydigit(buffer, v % 10, x + 4 * (w - i - 1), y);
    v /= 10;
  }
}

void faze_screen(uint8_t *buffer) {
  for (uint16_t i = 0; i < 1024; i++) {
    buffer[i] <<= prng() & 3;
  }
}

void clear_full_rect(uint8_t *buffer, MRect r) {
  r.y >>= 3;
  r.h >>= 3;
  for (uint8_t y = 0; y < r.h; y++) {
    for (uint8_t x = 0; x < r.w; x++) {
      buffer[x + r.x + (y + r.y) * WIDTH] = 0;
    }
  }
}
