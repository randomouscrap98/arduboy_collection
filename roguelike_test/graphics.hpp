#pragma once

#include "mymath.hpp"
#include <Arduboy2.h>

void print_tinydigit(uint8_t *buffer, uint8_t v, uint8_t x, uint8_t y);
void print_tinynumber(uint8_t *buffer, uint16_t v, uint8_t w, uint8_t x,
                      uint8_t y);
void faze_screen(uint8_t *buffer);
void clear_full_rect(uint8_t *buffer, MRect r);
