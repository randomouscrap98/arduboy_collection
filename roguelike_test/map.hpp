#pragma once

#include <Arduboy2.h>

#include "game.hpp"

constexpr uint8_t TILEDEFAULT = 1;
constexpr uint8_t TILEEXIT = 2;

// Each room element is 4 bytes FYI (Rect)
constexpr uint8_t ROOMSMAXDEPTH = 10;

#define RANDROOMDIM(config)                                                    \
  (config->room_min + random(1 + config->room_max - config->room_min))

void set_empty_map(Map m);
bool clear_rect_map(Map m, MRect r);

struct RoomConfig {
  uint8_t minwidth = 2;       // Each room's minimum dimension must be this.
  uint8_t doorbuffer = 1;     // Doors not generated this close to edge
  uint8_t maxwallretries = 3; // This changes the chance of "big" rooms
  uint8_t ninepool = 9;       // Larger = less likely
  uint8_t cubiclepool = 5;
  uint8_t cubiclemaxlength = 4;
  uint8_t bumppool = 5;
};

// Split rooms into smaller rooms randomly until a minimum is reached. If a room
// cannot be split but it's of certain dimensions, randomly add "interesting"
// features to it.
void genRoomsType(RoomConfig *config, Map m, MapPlayer *p);

struct Type1Config {
  // uint8_t startX = ;
  // uint8_t startY;
  uint8_t hw_stoppool = 50;
  uint8_t hw_cdpool = 4;
  uint8_t overlap = 0;      // 1 = hw overlap, 2 = room overlap, 3 = both ofc
  uint8_t room_pool = 3;    // Higher = less chance
  uint8_t room_retries = 3; // Once a spot is picked, how many times to try
  uint8_t room_min = 2;     // min room size
  uint8_t room_max = 4;     // max room size
  uint8_t door_buffer = 0;  // min distance into room for doorway
};

void gen_type_1(Type1Config *config, Map m, MapPlayer *p);

struct TileConfig {
  uint8_t perimeter;
  uint8_t main;
  uint8_t exit;
};

struct Type2Config {
  uint8_t stops = 10;
  // uint8_t turn_unlikely = 3;
  uint8_t room_unlikely = 6; // How unlikely a room is to spawn at a stop.
  uint8_t room_min = 2;      // min room size
  uint8_t room_max = 4;      // max room size
  uint8_t turn_unlikely = 255;
  TileConfig tiles;
};

void gen_type_2(Type2Config *config, Map m, MapPlayer *p);
