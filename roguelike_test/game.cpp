#include "game.hpp"

void cardinal_to_dir(uint8_t cardinal, int8_t *dx, int8_t *dy) {
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

float cardinal_to_rad(uint8_t cardinal) {
  switch (cardinal) {
  case 0: // North / up
    return PI * 0.5f;
  case 1: // East / right
    return 0;
  case 2: // South / down
    return PI * 1.5f;
  case 3:
  default: // Default west? eehhhh
    return PI;
  }
}

static struct prng_state pstate = {.a = 0xfa, .b = 0xee, .c = 0xee, .d = 0xee};

#define rot8(x, k) (((x) << (k)) | ((x) >> (8 - (k))))
uint8_t prng() { // jsf8, see https://filterpaper.github.io/prng.html#jsf8
  uint8_t e = pstate.a - rot8(pstate.b, 1);
  pstate.a = pstate.b ^ rot8(pstate.c, 4);
  pstate.b = pstate.c + pstate.d;
  pstate.c = pstate.d + e;
  return pstate.d = e + pstate.a;
}

void prng_seed(uint16_t seed) {
  pstate.a = 0xf1; // From https://filterpaper.github.io/prng.html#jsf8
  pstate.b = 0xee;
  // Just a handful of seeds out of 65,536 are bad (luckily)
  if (seed == 6332 || seed == 18002 || seed == 19503)
    pstate.b = 0xef;
  pstate.c = (seed >> 8) & 0xff;
  pstate.d = seed & 0xff;
}

// Copies internal pstate (4 bytes only, luckily)
prng_state prng_snapshot() { return pstate; }
void prng_restore(prng_state state) { pstate = state; }

// Allows access to a shuffled list of 65,536 values in a unique order per
// "lcg_seed". Useful for generating a list of further seeds for use in dungeon
// generation for a given player
uint16_t lcg_shuffle(uint16_t lcg_seed, uint16_t iteration) {
  // Player seed doesn't change and sets up a linear congruential generator as a
  // shuffle (period 2^16)
  uint16_t a = (lcg_seed & 0xF000) | 0x0805;
  uint16_t c = 2 * (lcg_seed & 0xFFF) + 1;
  // Iteration is just the Nth value from lcg:
  return a * iteration + c;
}

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

// uint8_t detect_move(Arduboy2Base *arduboy) {
//   uint8_t move = 0;
//   if (arduboy->pressed(UP_BUTTON)) {
//     move |= GS_MOVEFORWARD;
//   }
//   if (arduboy->pressed(DOWN_BUTTON)) {
//     move |= GS_MOVEBACKWARD;
//   }
//   if (arduboy->pressed(LEFT_BUTTON)) {
//     move |= GS_TURNLEFT;
//   }
//   if (arduboy->pressed(RIGHT_BUTTON)) {
//     move |= GS_TURNRIGHT;
//   }
//   return move;
// }

// Run movement, setting "next player"
uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy) {

  uint8_t move = 0; // gs->buffered_input;
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
    cmod = -1;
  }
  if (arduboy->pressed(RIGHT_BUTTON)) {
    move |= GS_TURNRIGHT;
    cmod = 1;
  }

  if (move) {
    int8_t dx, dy;
    gs->next_player = gs->player;
    cardinal_to_dir(gs->player.cardinal, &dx, &dy);
    gs->next_player.posX += pmod * dx;
    gs->next_player.posY += pmod * dy;
    uint8_t mt = MAPT(gs->map, gs->next_player.posX, gs->next_player.posY);
    if (!(mt == TILEEMPTY || (mt & TILEEXIT) == TILEEXIT)) {
      move &= ~(GS_MOVEBACKWARD | GS_MOVEFORWARD);
      gs->next_player = gs->player; // Undo what we did before
    }
    // This should always succeed
    gs->next_player.cardinal = (gs->next_player.cardinal + 4 + cmod) & 3;
  }

  return move;
}

void gs_tickstamina(GameState *gs) {
  if (gs->stamina) {
    gs->stamina--;
    if (gs->stamina && gs->health < 255) {
      gs->stamina--;
      gs->health++;
    }
  } else if (gs->health) {
    gs->health--;
  }
}

bool gs_exiting(GameState *gs) {
  return (MAPT(gs->map, gs->next_player.posX, gs->next_player.posY) &
          TILEEXIT) == TILEEXIT;
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

void gs_draw_map_near(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
                      uint8_t ys, uint8_t range) {
  for (int8_t y = -range; y <= range; y++) {
    for (int8_t x = -range; x <= range; x++) {
      uint8_t px = x + gs->player.posX;
      uint8_t py = y + gs->player.posY;
      if (px >= gs->map.width || py >= gs->map.height)
        continue;
      uint8_t mt = MAPT(gs->map, px, py);
      arduboy->drawPixel(xs + px, ys + gs->map.height - 1 - py,
                         mt ? WHITE : BLACK);
    }
  }
}

void gs_draw_map_player(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
                        uint8_t ys, uint8_t col) {
  arduboy->drawPixel(xs + gs->player.posX,
                     ys + gs->map.height - 1 - gs->player.posY, col);
}
