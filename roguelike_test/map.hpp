#pragma once

#include <Arduboy2.h>

constexpr uint8_t TILEEMPTY = 0;
constexpr uint8_t TILEDEFAULT = 1;
constexpr uint8_t TILEEXIT = 2;

// Each room element is 4 bytes FYI (Rect)
constexpr uint8_t ROOMSMAXDEPTH = 10;

struct Map {
  uint8_t *map;
  uint8_t width;
  uint8_t height;
};

struct MapPlayer {
  uint8_t posX;
  uint8_t posY;
  int8_t dirX;
  int8_t dirY;
};

#define MPLAYRAD(p)                                                            \
  (p.dirX == 1 ? 0 : p.dirX == -1 ? PI : p.dirY == 1 ? PI * 0.5f : PI * 1.5f)

// A rectangle starting at x, y and having side w, h
struct MRect {
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t w = 0;
  uint8_t h = 0;
};

#define MAPT(m, x, y) m.map[(x) + (y) * m.width]

struct RoomConfig {
  uint8_t minwidth = 2;       // Each room's minimum dimension must be this.
  uint8_t doorbuffer = 1;     // Doors not generated this close to edge
  uint8_t maxwallretries = 3; // This changes the chance of "big" rooms
  uint8_t ninepool = 9;       // Larger = less likely
  uint8_t cubiclepool = 5;
  uint8_t cubiclemaxlength = 4;
  uint8_t bumppool = 5;
};

void set_empty_map(Map m);
// North, east, south, west. Up right down left. North is positive y
void set_player_dir(MapPlayer *p, uint8_t cardinal);
// void player_face_emptiest(MapPlayer *p, Map m);
uint8_t get_player_bestdir(MapPlayer *p, Map m);
// void set_player_posdir(Map m, MapPlayer *p, uint8_t posX, uint8_t posY);

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
