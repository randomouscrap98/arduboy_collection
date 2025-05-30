// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

#include "game.hpp"
#include "fxdata/fxdata.h"
#include "mymath.hpp"
#include <ArduboyFX.h>

uint8_t get_player_bestdir(MapPlayer *p, Map m) {
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
    if (!(mt == TILEEMPTY || (mt & TILEEXITBITS) == TILEEXITBITS)) {
      move &= ~(GS_MOVEBACKWARD | GS_MOVEFORWARD);
      gs->next_player = gs->player; // Undo what we did before
    }
    // This should always succeed
    gs->next_player.cardinal = (gs->next_player.cardinal + 4 + cmod) & 3;
  }

  return move;
}

// FULL game restart, reset region, inventory, etc
void gs_restart(GameState *gs) {
  gs->region = 1;
  gs->region_floor = 0;
  // gs->total_floor = 0;
  gs->stamina = STARTSTAMHEALTH;
  gs->health = STARTSTAMHEALTH;
  gs->menu_pos = 0;
  gs->item_pos = 0;
  gs->item_top = 0;
  gs->max_items = STARTITEMS;
  gs->millis_start = millis();
  memset(gs->inventory, 0, sizeof(gs->inventory));
  // Just a test
  // gs->inventory[0].count = 1;
  // gs->inventory[0].item = 1;
  // gs->inventory[5].count = 1;
  // gs->inventory[5].item = 2;
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
          TILEEXITBITS) == TILEEXITBITS;
}

bool gs_dead(GameState *gs) { return gs->health == 0; }

uint8_t gs_item_cursor(GameState *gs, Arduboy2Base *arduboy, uint8_t *swapped) {
  uint8_t opos = gs->item_pos;
  int8_t mod = 0;
  if (arduboy->justPressed(DOWN_BUTTON)) {
    mod += ITEMSACROSS;
  }
  if (arduboy->justPressed(UP_BUTTON)) {
    mod -= ITEMSACROSS;
  }
  if (arduboy->justPressed(RIGHT_BUTTON)) {
    mod += 1;
  }
  if (arduboy->justPressed(LEFT_BUTTON)) {
    mod -= 1;
  }
  // The itempos ALWAYS moves
  gs->item_pos = (gs->item_pos + mod + gs->max_items) % gs->max_items;
  // The top moves based on if the cursor is outside the range
  if (gs->item_pos < gs->item_top)
    gs->item_top = ITEMSACROSS * (gs->item_pos / ITEMSACROSS);
  if (gs->item_pos >= gs->item_top + ITEMSPAGE)
    gs->item_top =
        ITEMSACROSS * (gs->item_pos / ITEMSACROSS) - (ITEMSPAGE - ITEMSACROSS);
  if ((arduboy->buttonsState() & ITEMS_SWAPBTN) == ITEMS_SWAPBTN) {
    if (opos != gs->item_pos) {
      InventorySlot temp = gs->inventory[gs->item_pos];
      gs->inventory[gs->item_pos] = gs->inventory[opos];
      gs->inventory[opos] = temp;
      *swapped = 1;
    }
  }
  return gs->item_pos - gs->item_top;
}

bool gs_has_item(GameState *gs, uint8_t pos) {
  return gs->inventory[pos].count;
}

bool gs_add_item(GameState *gs, uint8_t item) {
  // First, check stackable. If stackable, look for place to put it
  uint8_t maxstack;
  FX::readDataObject<uint8_t>(itemstacks + item, maxstack);
  if (maxstack > 1) {
    for (uint8_t i = 0; i < gs->max_items; i++) {
      if (gs->inventory[i].count < maxstack && gs->inventory[i].item == item) {
        gs->inventory[i].count++;
        return true;
      }
    }
  }
  // OK, just look for any empty slot
  for (uint8_t i = 0; i < gs->max_items; i++) {
    if (gs->inventory[i].count == 0) {
      gs->inventory[i].item = item;
      gs->inventory[i].count = 1;
      return true;
    }
  }
  return false;
}

void gs_remove_item(GameState *gs, uint8_t pos) {
  gs->inventory[pos].count = 0;
  gs->inventory[pos].item = 0;
}

bool gs_consume_item(GameState *gs, uint8_t pos) {
  if (gs->inventory[pos].count > 0) {
    gs->inventory[pos].count--;
    if (gs->inventory[pos].count == 0)
      gs->inventory[pos].item = 0;
    return true;
  }
  return false;
}

uint8_t gs_add_health(GameState *gs, int16_t amount) {
  int16_t result = gs->health + amount;
  if (result < 0)
    result = 0;
  if (result > BASESTAMHEALTH)
    result = BASESTAMHEALTH;
  gs->health = result;
  return gs->health;
}

uint8_t gs_add_stamina(GameState *gs, int16_t amount) {
  int16_t result = gs->stamina + amount;
  if (result < 0)
    result = 0;
  if (result > BASESTAMHEALTH)
    result = BASESTAMHEALTH;
  gs->stamina = result;
  return gs->stamina;
}

bool gs_add_health_changed(GameState *gs, int16_t amount) {
  uint8_t ohealth = gs->health;
  return gs_add_health(gs, amount) != ohealth;
}
bool gs_add_stamina_changed(GameState *gs, int16_t amount) {
  uint8_t ostam = gs->stamina;
  return gs_add_stamina(gs, amount) != ostam;
}
