#pragma once

#include "map.hpp"
#include <Arduboy2.h>

struct GameState {
  Map map;
  MapPlayer player;
  MapPlayer prev_player;
};

constexpr uint8_t GS_MOVEFORWARD = 1;
constexpr uint8_t GS_MOVEBACKWARD = 2;
constexpr uint8_t GS_TURNLEFT = 4;
constexpr uint8_t GS_TURNRIGHT = 8;

// ONLY perform player position/direction update based on inputs. No animations
// or turn updates etc. are handled, though the return value indicates if actual
// movement occurred.

// ONLY convert arduboy inputs into GS events. This is slightly more complicated
// than just polling on arduboy, since we MAY have some buffering enabled
uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy);

// Draw very simple 1:1 map at given location. Does not check if out of bounds
void gs_draw_map(GameState *gs, Arduboy2Base *arduboy, uint8_t x, uint8_t y);
