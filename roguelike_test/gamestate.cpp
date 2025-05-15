#include "gamestate.hpp"

uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy) {

  uint8_t move = 0;

  if (arduboy->pressed(UP_BUTTON))
    move |= GS_MOVEFORWARD;
  if (arduboy->pressed(DOWN_BUTTON))
    move |= GS_MOVEBACKWARD;
  if (arduboy->pressed(LEFT_BUTTON))
    move |= GS_TURNLEFT;
  if (arduboy->pressed(RIGHT_BUTTON))
    move |= GS_TURNRIGHT;

  return move;
}

void gs_draw_map(GameState *gs, Arduboy2Base *arduboy, uint8_t xs, uint8_t ys) {
  arduboy->fillRect(xs, ys, gs->map.width, gs->map.height, BLACK);
  for (int y = 0; y < gs->map.height; y++) {
    for (int x = 0; x < gs->map.width; x++) {
      if (MAPT(gs->map, x, gs->map.height - 1 - y)) {
        arduboy->drawPixel(xs + x, ys + y, WHITE);
      }
    }
  }
}
