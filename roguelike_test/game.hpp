#pragma once

#include <Arduboy2.h>

constexpr uint8_t TILEEMPTY = 0;
constexpr uint8_t DIRNORTH = 0;
constexpr uint8_t DIREAST = 1;
constexpr uint8_t DIRSOUTH = 2;
constexpr uint8_t DIRWEST = 3;

constexpr float DIRRADS[] = {PI * 0.5f, 0, PI * 1.5f, PI};

void cardinal_to_dir(uint8_t cardinal, int8_t *dx, int8_t *dy);

// A rectangle starting at x, y and having side w, h
struct MRect {
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t w = 0;
  uint8_t h = 0;
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

// #define MPLAYRAD(p) (p.cardinal == 1 ? 0 : p.dirX == -1 ? PI : p.dirY == 1 ?
// PI * 0.5f : PI * 1.5f)

// North, east, south, west. Up right down left. North is positive y
// void set_player_dir(MapPlayer *p, uint8_t cardinal);
uint8_t get_player_bestdir(MapPlayer *p, Map m);

struct GameState {
  Map map;
  MapPlayer player;
  MapPlayer prev_player;
};

constexpr uint8_t GS_MOVEFORWARD = 1;
constexpr uint8_t GS_MOVEBACKWARD = 2;
constexpr uint8_t GS_TURNLEFT = 4;
constexpr uint8_t GS_TURNRIGHT = 8;

// Attempt certain movement, returning which action was performed. The changes
// are applied immediately to the state.
uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy);

// Draw very simple 1:1 map at given location. Does not check if out of bounds
void gs_draw_map(GameState *gs, Arduboy2Base *arduboy, uint8_t x, uint8_t y);
