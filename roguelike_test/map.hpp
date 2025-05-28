#pragma once

#include <Arduboy2.h>

#include "game.hpp"
#include "mymath.hpp"

// #define RANDROOMDIM(config)                                                    \
//   (config->room_min + random(1 + config->room_max - config->room_min))
#define DISTSQRD(x1, y1, x2, y2)                                               \
  (((x2) - (x1)) * ((x2) - (x1)) + ((y2) - (y1)) * ((y2) - (y1)))

void set_empty_map(Map m);
bool clear_rect_map(Map m, MRect r);
void remove_reserved_map(Map m);
bool has_2x2_box_around(Map m, uint8_t x, uint8_t y);

struct TileExtra {
  uint8_t tile;
  uint8_t unlikely;
  uint8_t type;
};

constexpr uint8_t TILEEXTRATYPE_NORMAL = 1;
constexpr uint8_t TILEEXTRATYPE_NOCORNER = 2;
constexpr uint8_t TILEEXTRATYPE_PILLAR = 3;

constexpr uint8_t TILEEXTRAMAX = 8;

/* clang-format off */
constexpr int8_t PERIMETERBUF[] PROGMEM =
  {1, 1, 1, 0, -1, -1, -1, 0};
// {
//   -1, 0,
//   -1, -1,
//   0, -1,
//   1, -1,
//   1, 0,
//   1, 1,
//   0, 1,
//   -1, 1,
// };
  //1, 1, 1, 0, -1, -1, -1, 0};
/* clang-format on */
constexpr uint8_t PERIMETERYOFS = 6;
// constexpr uint8_t PERIMETERCORNEROFS = 1;

struct TileConfig {
  uint8_t perimeter;
  uint8_t main;
  uint8_t exit;
  TileExtra extras[TILEEXTRAMAX];
  uint8_t extras_count;
};

struct Type2Config {
  // uint8_t turn_unlikely = 3;
  // prng16_state *state;
  // uint8_t stops = 10;
  // uint8_t room_unlikely = 6; // How unlikely a room is to spawn at a stop.
  // uint8_t room_min = 2;      // min room size
  // uint8_t room_max = 4;      // max room size
  // uint8_t turn_unlikely = 255;
  uint8_t stops;
  uint8_t room_unlikely; // How unlikely a room is to spawn at a stop.
  uint8_t room_min;      // min room size
  uint8_t room_max;      // max room size
  uint8_t turn_unlikely;
  TileConfig tiles;
};

void gen_type_2(Type2Config *config, Map m, MapPlayer *p);
