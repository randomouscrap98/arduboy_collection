#pragma once

#include <stdint.h>
#include <avr/pgmspace.h>

constexpr uint8_t wallTileWidth = 8;
constexpr uint8_t wallTileHeight = 8;

constexpr uint8_t wallTile[] PROGMEM
{
  wallTileWidth, wallTileHeight,

  //Frame 0
  0x18, 0xDB, 0xC3, 0xDB, 0x18, 0xDB, 0xC3, 0xDB
};
