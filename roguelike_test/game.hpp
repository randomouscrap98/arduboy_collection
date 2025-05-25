#pragma once

#include <Arduboy2.h>

constexpr uint8_t TILEEMPTY = 0;
constexpr uint8_t TILEEXIT = 15;
constexpr uint8_t MAXITEMS = 30;
constexpr uint8_t BASESTAMHEALTH = 255;
constexpr uint8_t STARTSTAMHEALTH = 3;

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

struct SaveGame {
  uint16_t player_seed;
  uint16_t total_runs;
  uint16_t total_wins;
  uint16_t total_rooms; // may run out? nobody will play it that much...
  uint16_t total_items; // picked up
  uint16_t total_used;  // items
  uint16_t total_kills;
  uint32_t total_seconds; // played in dungeon
  uint16_t completed_region[4];
};

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
  uint8_t menu_pos; // global menu pos from last menu
  uint8_t inventory[MAXITEMS];
  unsigned long millis_start;
  // uint8_t buffered_input; // What it says
};

constexpr uint8_t GS_MOVEFORWARD = 1;
constexpr uint8_t GS_MOVEBACKWARD = 2;
constexpr uint8_t GS_TURNLEFT = 4;
constexpr uint8_t GS_TURNRIGHT = 8;

constexpr uint8_t GS_STATEMAIN = 1;
constexpr uint8_t GS_STATEANIMATE = 2;
constexpr uint8_t GS_FLOORTRANSITION = 3;
constexpr uint8_t GS_STATEMENUANIM = 4;
constexpr uint8_t GS_STATEMENU = 5;
constexpr uint8_t GS_STATEGAMEOVER = 6;
constexpr uint8_t GS_STATEABOUT = 7;

// Attempt certain movement, returning which action was performed. The changes
// are applied immediately to the state.
uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy);

void gs_restart(GameState *gs);

// Drain stamina, heal health or hurt it.
void gs_tickstamina(GameState *gs);

bool gs_exiting(GameState *gs);
bool gs_dead(GameState *gs);

// Draw very simple 1:1 map at given location. Does not check if out of bounds
void gs_draw_map(GameState *gs, Arduboy2Base *arduboy, uint8_t x, uint8_t y);
void gs_draw_map_near(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
                      uint8_t ys, uint8_t range);
void gs_draw_map_player(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
                        uint8_t ys, uint8_t col);
