#pragma once

#include <Arduboy2.h>

constexpr uint8_t ITEMSACROSS = 3;
constexpr uint8_t ITEMSDOWN = 3;
constexpr uint8_t ITEMSPAGE = ITEMSACROSS * ITEMSDOWN;

constexpr uint8_t TILEEMPTY = 0;
constexpr uint8_t TILEEXITBITS = 15;
constexpr uint8_t TILERESERVED = 254;
// tile that's unusable, used for special things
constexpr uint8_t MAXITEMS = 27;
constexpr uint8_t BASESTAMHEALTH = 255;
constexpr uint8_t STARTSTAMHEALTH = 255;
constexpr uint8_t STARTITEMS = ITEMSPAGE;

// constexpr uint8_t ITEMS_SWAPBTN = A_BUTTON | B_BUTTON;
constexpr uint8_t ITEMS_SWAPBTN = B_BUTTON;

// ITEM DEFINITIONS
constexpr uint8_t ITEM_POTION = 1;
constexpr uint8_t ITEM_HIPOTION = 2;
constexpr uint8_t ITEM_FOOD = 3;
constexpr uint8_t ITEM_HIFOOD = 4;
constexpr uint8_t ITEM_ROCK = 5;
constexpr uint8_t ITEM_SHARPROCK = 6;
constexpr uint8_t ITEM_REVIVE = 7;
constexpr uint8_t ITEM_HIREVIVE = 8;
constexpr uint8_t ITEM_ATTACK = 9;
constexpr uint8_t ITEM_DEFENSE = 10;
constexpr uint8_t ITEM_BADSWORD = 11;
constexpr uint8_t ITEM_SWORD = 12;
constexpr uint8_t ITEM_GREATSWORD = 13;

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

struct InventorySlot {
  uint8_t item;
  uint8_t count;
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
  // uint8_t total_floor;
  uint8_t stamina;
  uint8_t health;
  uint8_t menu_pos;  // global menu pos from last menu
  uint8_t item_top;  // top of visible item window in item count
  uint8_t item_pos;  // actual position within inventory
  uint8_t max_items; // Max items for current run (can increase)
  uint8_t tempstate1;
  InventorySlot inventory[MAXITEMS];
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
constexpr uint8_t GS_STATEITEMMENU = 8;
constexpr uint8_t GS_STATEITEMSELECT = 9;

// Attempt certain movement, returning which action was performed. The changes
// are applied immediately to the state.
uint8_t gs_move(GameState *gs, Arduboy2Base *arduboy,
                bool (*extra_check)(GameState *, uint8_t, uint8_t));

void gs_restart(GameState *gs);

// Drain stamina, heal health or hurt it.
void gs_tickstamina(GameState *gs);

bool gs_exiting(GameState *gs);
bool gs_dead(GameState *gs);

// Compute new cursor based on button presses. Returns relative 1d pos "on
// screen". Top left = 0, bottom right = ITEMSPAGE - 1
uint8_t gs_item_cursor(GameState *gs, Arduboy2Base *arduboy, uint8_t *swapped);

bool gs_add_item(GameState *gs, uint8_t item);
bool gs_has_item(GameState *gs, uint8_t pos);
void gs_remove_item(GameState *gs, uint8_t pos);
bool gs_consume_item(GameState *gs, uint8_t pos);
uint8_t gs_add_health(GameState *gs, int16_t amount);
uint8_t gs_add_stamina(GameState *gs, int16_t amount);
bool gs_add_health_changed(GameState *gs, int16_t amount);
bool gs_add_stamina_changed(GameState *gs, int16_t amount);

// Draw very simple 1:1 map at given location. Does not check if out of bounds
// void gs_draw_map(GameState *gs, Arduboy2Base *arduboy, uint8_t x, uint8_t y);
// void gs_draw_map_near(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
//                       uint8_t ys, uint8_t range);
// void gs_draw_map_player(GameState *gs, Arduboy2Base *arduboy, uint8_t xs,
//                         uint8_t ys, uint8_t col);
