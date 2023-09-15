#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

constexpr uint8_t lightWidth = 100;
constexpr uint8_t lightHeight = 32;

constexpr uint8_t light[] PROGMEM
{
  lightWidth, lightHeight,

  //Frame 0
  0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x88, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 
  0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA0, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xAA, 0x00, 0x88, 0x00, 0xA0, 0x00, 0x88, 0x00, 0xA2, 0x00, 0x88, 0x00, 0xA2, 0x00, 
  0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 0xAA, 0x40, 0xAA, 0x00, 0xA8, 0x40, 0xAA, 0x00, 
  0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x51, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x50, 0xAA, 0x44, 0xAA, 0x50
};
