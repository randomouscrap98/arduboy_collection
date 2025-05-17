#include "game.hpp"

void cardinal_to_dir(uint8_t cardinal, float *dx, float *dy) {
  *dx = 0;
  *dy = 0;
  switch (cardinal) {
  case 0: // North / up
    *dy = 1;
    return;
  case 1: // East / right
    *dx = 1;
    return;
  case 2: // South / down
    *dy = -1;
    return;
  case 3:
  default: // Default west? eehhhh
    *dx = -1;
    return;
  }
}
// void set_player_dir(MapPlayer *p, uint8_t cardinal) {
// }

uint8_t get_player_bestdir(MapPlayer *p, Map m) {
  // uint8_t thi[4] = {0};
  uint8_t max_count = 0;
  uint8_t max_dir = 0;
  uint8_t x, y;
  int8_t dx, dy;
  for (uint8_t i = 0; i < 4; i++) {
    x = p->posX;
    y = p->posY;
    cardinal_to_dir(i, &dx, &dy);
    uint8_t count = 0;
    while (1) {
      x += dx;
      y += dy;
      if (MAPT(m, x, y) != TILEEMPTY) {
        break;
      }
      count++;
    }
    if (count > max_count) {
      max_dir = i;
      max_count = count;
    }
  }
  return max_dir;
}

uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy) {

  uint8_t move = 0;
  int8_t pmod = 0;
  int8_t cmod = 0;

  if (arduboy->pressed(UP_BUTTON)) {
    move |= GS_MOVEFORWARD;
    pmod = 1;
  }
  if (arduboy->pressed(DOWN_BUTTON)) {
    move |= GS_MOVEBACKWARD;
    pmod = -1;
  }
  if (arduboy->pressed(LEFT_BUTTON)) {
    move |= GS_TURNLEFT;
  }
  if (arduboy->pressed(RIGHT_BUTTON)) {
    move |= GS_TURNRIGHT;
  }

  // if (move) {
  //   gs->prev_player = gs->player;
  //   gs->player.posX += pmod * gs->prev_player.dirX;
  //   gs->player.posY += pmod * gs->prev_player.dirY;
  // }

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
