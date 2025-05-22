#pragma once

#include <Arduboy2.h>

constexpr uint8_t TILEEMPTY = 0;
constexpr uint8_t TILEEXIT = 15;
constexpr uint8_t DIRNORTH = 0;
constexpr uint8_t DIREAST = 1;
constexpr uint8_t DIRSOUTH = 2;
constexpr uint8_t DIRWEST = 3;

void cardinal_to_dir(uint8_t cardinal, int8_t *dx, int8_t *dy);
float cardinal_to_rad(uint8_t cardinal);

// struct prng16_state {
//   uint16_t a, b, c, d;
// };

struct prng_state {
  uint8_t a, b, c, d;
};

uint8_t prng();
void prng_seed(uint16_t seed);
prng_state prng_snapshot();
void prng_restore(prng_state state);

// uint16_t prng16(prng16_state *s);
// void prng16_init(prng16_state *x, uint16_t seed);

#define RANDB(x) (prng() % (x))

// A rectangle starting at x, y and having side w, h
struct MRect {
  uint8_t x, y, w, h;
};

struct Map {
  uint8_t *map;
  uint8_t width;
  uint8_t height;
};

#define MAPT(m, x, y) m.map[(x) + (y) * m.width]

struct MapPlayer {
  uint8_t posX;
  uint8_t posY;
  uint8_t cardinal;
};

// North, east, south, west. Up right down left. North is positive y
// void set_player_dir(MapPlayer *p, uint8_t cardinal);
uint8_t get_player_bestdir(MapPlayer *p, Map m);

struct GameState {
  Map map;
  MapPlayer player;
  MapPlayer next_player;
  uint8_t state;      // current game state
  uint8_t animend;    // total frames for this animation
  uint8_t animframes; // remaining animation frames
  uint8_t region;
  uint8_t region_floor;
  uint8_t total_floor;
  uint8_t stamina;
  uint8_t health;
  // uint8_t buffered_input; // What it says
};

constexpr uint8_t GS_MOVEFORWARD = 1;
constexpr uint8_t GS_MOVEBACKWARD = 2;
constexpr uint8_t GS_TURNLEFT = 4;
constexpr uint8_t GS_TURNRIGHT = 8;

constexpr uint8_t GS_STATEMAIN = 1;
constexpr uint8_t GS_STATEANIMATE = 2;
constexpr uint8_t GS_FLOORTRANSITION = 3;

// Attempt certain movement, returning which action was performed. The changes
// are applied immediately to the state.
uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy);

// Drain stamina, heal health or hurt it.
void gs_tickstamina(GameState *gs);

bool gs_exiting(GameState *gs);

// Draw very simple 1:1 map at given location. Does not check if out of bounds
void gs_draw_map(GameState *gs, Arduboy2Base *arduboy, uint8_t x, uint8_t y);
void gs_draw_map_near(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
                      uint8_t ys, uint8_t range);
void gs_draw_map_player(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
                        uint8_t ys, uint8_t col);
