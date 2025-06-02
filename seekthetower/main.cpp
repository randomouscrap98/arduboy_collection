// This saves 1000 bytes but doesn't help the overdraw glitch
// #define RCSMALLLOOPS

#include <Arduboy2.h>
#include <Tinyfont.h>
#include <avr/pgmspace.h>

// This saves ~2700 pgm and 110 ram, will need exitToBootloader in menu
ARDUBOY_NO_USB

// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

// #define FULLMAP
// #define INSTANTFLOORUP
// #define INSTANTREFRESH
// #define RCNOBACKGROUND

#include "LocalRaycast/ArduboyRaycastFX.h"
#include "game.hpp"
#include "graphics.hpp"
#include "map.hpp"
#include "mymath.hpp"
#include "sound.hpp"

// Resources
#include "fxdata/fxdata.h"

Arduboy2Base arduboy;
Tinyfont tinyfont =
    Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint8_t NUMINTERNALBYTES = 8;
constexpr uint8_t NUMSPRITES = 16;
constexpr uint8_t NUMOWSPRITES = 14;

constexpr uflot HALF = 0.5f;   // IDK
constexpr muflot MHALF = 0.5f; // IDK

constexpr float FOV = 1.0f;
constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 4.25f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

constexpr uint8_t DEFAULTANIMFRAMES = ((float)FRAMERATE) * 0.34f;
constexpr uint8_t DEFAULTANIMEXITFRAMES = ((float)FRAMERATE) * 0.6f;
constexpr uint8_t DEFAULTANIMGAMEOVERFRAMES = ((float)FRAMERATE);

// Once again we pick 16 sprites just in case we need them. 16 is a decent
// number to not take up all the memory but still have enough to work with.
RcContainer<NUMSPRITES, NUMINTERNALBYTES, WIDTH - SIDESIZE, HEIGHT - BOTTOMSIZE>
    raycast(tilesheet, spritesheet, spritesheetMask);

GameState gs;
SaveGame sg;
prng_state gen_state;

void print_current_item() {
  prep_textarea();
  print_item_info(gs.inventory[gs.item_pos]);
}

void initiate_floor_transition() {
  sg.total_rooms++;
  gs.animframes = 0; // begin animation at 0 (?)
  gs.state = GS_FLOORTRANSITION;
  gs.animend = DEFAULTANIMEXITFRAMES;
  descend_beep();
}

void initiate_animation() {
  gs.animframes = 0; // begin animation at 0 (?)
  gs.state = GS_STATEANIMATE;
  gs.animend = DEFAULTANIMFRAMES;
}

void initiate_gameover() {
  sg.total_seconds += (millis() - gs.millis_start) / 1000;
  // Save current game state on game over
  FX::saveGameState(sg);
  gs.animframes = 0; // begin animation at 0 (?)
  gs.state = GS_STATEGAMEOVER;
  gs.animend = DEFAULTANIMGAMEOVERFRAMES; // Reuse exit frames
  raycast.render.setLightIntensity(2.0);
}

void initiate_mainmenu() {
  gs.animframes = 0; // begin animation at 0 (?)
  gs.animend = DEFAULTANIMFRAMES;
  gs.state = GS_STATEMENUANIM;
}

void initiate_about() { gs.state = GS_STATEABOUT; }
void initiate_itemmenu() {
  gs.tempstate1 = 1;
  gs.state = GS_STATEITEMMENU;
}
void initiate_gamemain() {
  draw_runtime_data(&gs); // not STRICTLY necessary but just in case...
  draw_items_menu(&gs);
  gs.state = GS_STATEMAIN;
}
void initiate_itemselect() { gs.state = GS_STATEITEMSELECT; }

void begin_game() {
  // void __attribute__((noinline)) begin_game() {
  //  This indicates the game started without a save game. We separate this from
  //  the point of checking for the save because we apparently need some
  //  randomness to the time before calling generateRandomSeed
  if (sg.player_seed == 0) { // Sorry, the 0 seed is reserved I guess...
    sg.player_seed = (uint16_t)arduboy.generateRandomSeed();
  }
  sg.total_runs++;
  FX::saveGameState(sg);
  gs_restart(&gs);
  prng_seed(lcg_shuffle(sg.player_seed, sg.total_runs)); // change this later
  gen_state = prng_snapshot();
  initiate_floor_transition();
}

constexpr uint8_t IDENTITYBYTE = 0; // The base identifier for all things
constexpr uint8_t POSBYTE =
    1; // Realistically need an actual tile position within the map
constexpr uint8_t ANIMWAITBYTE = 2;
constexpr uint8_t DIRBYTE = 3; // Facing direction for enemies (if they use it)
constexpr uint8_t QUEUEBYTE = 4;  // Queued up action (usually attack)
constexpr uint8_t NEWPOSBYTE = 5; // Next position for animation (movement)

// #define ENCODENEWPOS(s, x, y) (s)->intstate[NEWPOSBYTE] = (((y << 4)) | (x))
// #define DECODENEWPOSX(s) ((s)->intstate[NEWPOSBYTE] & 15)
// #define DECODENEWPOSY(s) (((s)->intstate[NEWPOSBYTE] >> 4) & 15)
#define ENCODEBPOS(x, y) (((y << 4)) | (x))
#define DECODEBPOSX(p) (p & 15)
#define DECODEBPOSY(p) ((p >> 4) & 15)

constexpr uint8_t G_ITEMBAG = 1;
constexpr uint8_t G_ROCK = 2;
constexpr uint8_t G_MOUSE = 16;

// Update animation rotation/position based on given delta between current
// position and new player position. Will NOT be extended to enemies, since
// they don't have rotation and their position is different
void update_visual_positions(float delta) {
  float indelt = 1 - delta;
  raycast.player.posX =
      0.5f + gs.player.posX * (indelt) + delta * gs.next_player.posX;
  raycast.player.posY =
      0.5f + gs.player.posY * (indelt) + delta * gs.next_player.posY;
  float c1 = cardinal_to_rad(gs.player.cardinal);
  float c2 = cardinal_to_rad(gs.next_player.cardinal);
  // There are certain turns which go the long way. Fix that.
  if (fabs(c2 - c1) > 2) {
    if (c2 > c1) {
      c2 -= 2 * PI;
    } else {
      c1 -= 2 * PI;
    }
  }
  raycast.player.initPlayerDirection((float)indelt * c1 + (float)delta * c2,
                                     FOV);
  // Might as well also do enemies movement
  for (uint8_t i = 0; i < NUMSPRITES; i++) {
    RcSprite<NUMINTERNALBYTES> *sprite = &raycast.sprites.sprites[i];
    if (!ISSPRITEACTIVE((*sprite)))
      continue;
    uint8_t sx = DECODEBPOSX(sprite->intstate[POSBYTE]);
    uint8_t sy = DECODEBPOSY(sprite->intstate[POSBYTE]);
    uint8_t nx = DECODEBPOSX(sprite->intstate[NEWPOSBYTE]);
    uint8_t ny = DECODEBPOSY(sprite->intstate[NEWPOSBYTE]);
    sprite->x = 0.5f + sx * indelt + nx * delta;
    sprite->y = 0.5f + sy * indelt + ny * delta;
  }
}

// Move sprites to their next position
void sprites_to_next_position() {
  for (uint8_t i = 0; i < NUMSPRITES; i++) {
    RcSprite<NUMINTERNALBYTES> *sprite = &raycast.sprites.sprites[i];
    if (!ISSPRITEACTIVE((*sprite)))
      continue;
    sprite->intstate[POSBYTE] = sprite->intstate[NEWPOSBYTE];
  }
}

void run_menu() {
  constexpr uint8_t MENUOPTIONS = 4;
  uint8_t opos = gs.menu_pos;
  gs.menu_pos =
      (gs.menu_pos + MENUOPTIONS + (arduboy.justPressed(DOWN_BUTTON) ? 1 : 0) -
       (arduboy.justPressed(UP_BUTTON) ? 1 : 0)) %
      MENUOPTIONS;
  if (opos != gs.menu_pos) {
    menu_move_beep();
  }
  display_menu(gs.menu_pos);
  if (arduboy.justPressed(A_BUTTON)) {
    switch (gs.menu_pos) {
    case 0:
      // Need to do a BIT more...
      begin_game();
      break;
    case 1:
      // Don't save it (for now)
      arduboy.audio.toggle();
      menu_select_beep();
      break;
    case 2:
      initiate_about();
      menu_select_beep();
      break;
    case 3:
      // Can exit immediately
      arduboy.audio.saveOnOff(); // Might as well
      arduboy.exitToBootloader();
      break;
    }
  }
}

void run_about() {
  display_about(&sg);
  if (arduboy.justPressed(A_BUTTON)) {
    confirm_beep();
    initiate_mainmenu();
  }
}

void run_itemmenu() {
  if (arduboy.justPressed(A_BUTTON) && gs_has_item(&gs, gs.item_pos)) {
    menu_select_beep();
    gs.tempstate1 = 1; // Don't let b cancel the whole menu
    gs.menu_pos = 0;
    initiate_itemselect();
  }
  if (arduboy.justReleased(B_BUTTON)) {
    if (gs.tempstate1) {
      gs.tempstate1 = 0;
    } else {
      initiate_gamemain();
      item_close_beep();
      prep_textarea(); // clear it
      return;
    }
  }
  // NOTE: this keeps the item position from before...
  uint8_t opos = gs.item_pos;
  uint8_t vpos = gs_item_cursor(&gs, &arduboy, &gs.tempstate1);
  if (opos != gs.item_pos) {
    menu_move_beep();
  }
  // is this too laggy? IDK...
  print_current_item();
  draw_items_menu_w_cursor(
      &gs, vpos, (arduboy.buttonsState() & ITEMS_SWAPBTN) == ITEMS_SWAPBTN);
}

void run_itemselect() {
  uint8_t opos = gs.menu_pos;
  if (arduboy.justPressed(B_BUTTON)) {
    cancel_beep();
    initiate_itemmenu();
  }
  if (arduboy.justPressed(A_BUTTON)) {
    // confirm_beep();
    if (gs.menu_pos == 1) { // Toss, it's easier
      toss_beep();
      gs_remove_item(&gs, gs.item_pos);
      initiate_itemmenu();
      gs.tempstate1 = 0; // Don't want to enter temp state (hack!!)
    } else {
      bool used = true;
      switch (gs.inventory[gs.item_pos].item) {
      case ITEM_POTION:
        used = gs_add_health_changed(&gs, 128);
        break;
      case ITEM_HIPOTION:
        used = gs_add_health_changed(&gs, 255);
        break;
      case ITEM_FOOD:
        used = gs_add_stamina_changed(&gs, 128);
        break;
      case ITEM_HIFOOD:
        used = gs_add_stamina_changed(&gs, 255);
        break;
      default:
        used = false;
        break;
      }
      print_current_item();
      // It just HAPPENS to be the same...
      tinyfont.setCursor(BMESSAGEX + 82, BMESSAGEY);
      if (used) {
        confirm_beep();
        gs_consume_item(&gs, gs.item_pos);
        tinyfont.print(F("USED 1"));
      } else {
        cancel_beep();
        tinyfont.print(F("NO EFFECT"));
      }
      initiate_gamemain(); // kinda sucks but exit to game...
    }
    return;
  }
  if (arduboy.justPressed(RIGHT_BUTTON)) {
    gs.menu_pos = 1;
  }
  if (arduboy.justPressed(LEFT_BUTTON)) {
    gs.menu_pos = 0;
  }
  if (opos != gs.menu_pos) {
    menu_move_beep();
  }
  print_current_item();
  tinyfont.setCursor(BMESSAGEX + 82, BMESSAGEY);
  tinyfont.print(F("USE  TOSS"));
  // uint8_t cpos = BMESSAGEX + 76 * gs.menu_pos;
  tinyfont.setCursor(BMESSAGEX + 77 + 25 * gs.menu_pos, BMESSAGEY);
  tinyfont.print(F("#"));
}

void gen_region(uint8_t region) {
  Type2Config c;
  // c.state = &rngstate;
  //  For now, these are constants (but may change per region)
  raycast.render.setLightIntensity(1.5); // 2.0
  raycast.render.spriteShading = RcShadingType::Black;
  switch (region) {
  case 1:
  default:
    // memcpy_P(&c, &REGION_DUNGEON, sizeof(Type2Config));
    // c = memcpy_P();
    c.stops = 10;
    c.room_unlikely = 3;
    c.room_min = 2;
    c.room_max = 4;
    // c.room_unlikely = 1; // For big rooms (debugging?)
    // c.room_min = 3;
    // c.room_max = 4;
    c.turn_unlikely = 255;
    c.tiles.main = 1;
    c.tiles.perimeter = 7;
    c.tiles.exit = 15;
    c.tiles.extras_count = 3;
    c.tiles.extras[0].tile = 2;
    c.tiles.extras[0].type = TILEEXTRATYPE_NORMAL;
    c.tiles.extras[0].unlikely = 10;
    c.tiles.extras[1].tile = 3;
    c.tiles.extras[1].type = TILEEXTRATYPE_NOCORNER;
    c.tiles.extras[1].unlikely = 2; // It's fun to have a lot of these?
    c.tiles.extras[2].tile = 4;
    c.tiles.extras[2].type = TILEEXTRATYPE_PILLAR;
    c.tiles.extras[2].unlikely = 20;
    gen_type_2(&c, gs.map, &gs.player);
    break;
  }
}

// Refresh the full menu background, the region digits, and the local map
// around the player.
void draw_menu_init() {
  render_fximage(menu, arduboy.sBuffer, WIDTH, HEIGHT);
  print_tinydigit(arduboy.sBuffer, gs.region, 113, 20);
  print_tinynumber(arduboy.sBuffer, gs.region_floor, 2, 120, 20);
#ifdef FULLMAP
  draw_full_map(&gs, MAPX, MAPY);
#endif
  draw_items_menu(&gs);
}

void item_hover(RcSprite<NUMINTERNALBYTES> *sp) {
  sp->setHeight(4 + 2 * sin(arduboy.frameCount / 6.0f));
}

void anim_normal(RcSprite<NUMINTERNALBYTES> *sp) {
  sp->frame = sp->intstate[IDENTITYBYTE] +
              ((arduboy.frameCount >> sp->intstate[ANIMWAITBYTE]) & 3);
}

RcSprite<NUMINTERNALBYTES> *sprite_in_tile(uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < NUMSPRITES; i++) {
    RcSprite<NUMINTERNALBYTES> *sprite = &raycast.sprites.sprites[i];
    if (!ISSPRITEACTIVE((*sprite)))
      continue;
    if (DECODEBPOSX(sprite->intstate[POSBYTE]) == x &&
        DECODEBPOSY(sprite->intstate[POSBYTE]) == y) {
      return sprite;
    }
  }
  return NULL;
}

// Line of sight with range. Returns the direction to face
int8_t line_of_sight(RcSprite<NUMINTERNALBYTES> *sprite, uint8_t x, uint8_t y,
                     uint8_t range) {
  uint8_t sx = DECODEBPOSX(sprite->intstate[POSBYTE]);
  uint8_t sy = DECODEBPOSY(sprite->intstate[POSBYTE]);
  int8_t result = -1;
  if (sx == x) {
    if (sy > y) {
      uint8_t t = sy;
      sy = y;
      y = t;
      result = DIRSOUTH;
    } else {
      result = DIRNORTH;
    }
    if (y - sy > range) {
      return -1;
    }
    for (; sy <= y; sy++) {
      if (MAPT(gs.map, x, sy) != TILEEMPTY) {
        return -1;
      }
    }
  } else if (sy == y) {
    if (sx > x) {
      uint8_t t = sx;
      sx = x;
      x = t;
      result = DIRWEST; // Sprite is to the right of player
    } else {
      result = DIREAST;
    }
    if (x - sx > range) {
      return -1;
    }
    for (; sx <= x; sx++) {
      if (MAPT(gs.map, sx, y) != TILEEMPTY) {
        return -1;
      }
    }
  }
  return result;
}

void mouse_ai(RcSprite<NUMINTERNALBYTES> *sprite) {
  int8_t dx, dy, pdir;
  // Mice can only attack in plus sign. Reuse the line of sight func
  // to see if the NEW position is somewhere we can immediately attack
  pdir = line_of_sight(sprite, gs.next_player.posX, gs.next_player.posY, 1);
  if (pdir >= 0) {
    // Queue attack, no movement
    sprite->intstate[DIRBYTE] = pdir;
    sprite->intstate[QUEUEBYTE] = 1;
    return;
  }
  // If it sees the player, immediately face the player first
  // before moving. Use the OLD player position (it might make more sense,
  // consider corners, etc)
  pdir = line_of_sight(sprite, gs.player.posX, gs.player.posY, 4);
  if (pdir >= 0) {
    sprite->intstate[DIRBYTE] = pdir;
  }
  cardinal_to_dir(sprite->intstate[DIRBYTE], &dx, &dy);
  uint8_t nx = DECODEBPOSX(sprite->intstate[POSBYTE]) + dx;
  uint8_t ny = DECODEBPOSY(sprite->intstate[POSBYTE]) + dy;
  //  If the AI is about to run into a wall or literally going to bump INTO
  //  another enemy, pick a new direction and run the AI again.
  //  At most it should call itself 3 times, so the stack isn't bad
  if (MAPT(gs.map, nx, ny) != TILEEMPTY || sprite_in_tile(nx, ny)) {
    //(nx == gs.next_player.posX && ny == gs.next_player.posY)) {
    sprite->intstate[DIRBYTE] = (sprite->intstate[DIRBYTE] + 1) & 3;
    mouse_ai(sprite);
  }
  // Now, encode the new position, since it's good
  sprite->intstate[NEWPOSBYTE] = ENCODEBPOS(nx, ny);
}

// Do a tick of the entire world. Enemies, items, etc. Use NEW player
// position when checking
void world_tick() {
  for (uint8_t i = 0; i < NUMSPRITES; i++) {
    RcSprite<NUMINTERNALBYTES> *sprite = &raycast.sprites.sprites[i];
    if (!ISSPRITEACTIVE((*sprite)))
      continue;
    switch (sprite->intstate[IDENTITYBYTE]) {
    case G_MOUSE:
      mouse_ai(sprite);
      break;
    }
  }
}

struct SpriteDefinition {
  uint8_t size;
  uint8_t offset;
  uint8_t animwait;
  void (*func)(RcSprite<NUMINTERNALBYTES> *);
};

constexpr SpriteDefinition sprite_definitions[] PROGMEM = {
    {},
    {
        2,
        0,
        1,
        item_hover,
    },
    {
        3,
        4,
        1,
        NULL,
    },
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {},
    {
        2,
        4,
        2,
        anim_normal,
    },
};

void setup_sprite(uint8_t graphic, uint8_t x, uint8_t y) {
  SpriteDefinition sp;
  memcpy_P(&sp, sprite_definitions + graphic, sizeof(SpriteDefinition));
  RcSprite<NUMINTERNALBYTES> *sprite =
      raycast.sprites.addSprite(0, 0, graphic, sp.size, sp.offset, sp.func);
  // Most things will use this (other than items)
  sprite->intstate[IDENTITYBYTE] = graphic;
  sprite->intstate[POSBYTE] = ENCODEBPOS(x, y);
  sprite->intstate[NEWPOSBYTE] = sprite->intstate[POSBYTE];
  sprite->intstate[ANIMWAITBYTE] = sp.animwait;
  sprite->intstate[DIRBYTE] = prng() & 3; // IDK, random direction
  // Pick a random item to fill the given graphic. We reuse graphics for various
  // items, because our sprite pool is only 256, same as the item pool, and we
  // want enemies and stuff obviously
  switch (graphic) {
  case G_ITEMBAG:
    if (RANDOF(16)) {
      sprite->intstate[IDENTITYBYTE] = ITEM_REVIVE; // Revive spirit
    } else {
      sprite->intstate[IDENTITYBYTE] =
          RANDOF(3) ? ITEM_POTION : ITEM_FOOD; // one in three chance for potion
    }
    break;
  case G_ROCK:
    sprite->intstate[IDENTITYBYTE] = ITEM_ROCK; // rock
    break;
  case G_MOUSE:
    break;
  }
}

struct SpriteLimiter {
  uint8_t current;
  uint8_t limit;
};

bool any_empty(Map map, uint8_t x, uint8_t y) {
  return MAPT(map, x, y) == TILEEMPTY;
}

uint8_t try_spawn(uint8_t graphic, uint8_t retries, uint8_t count,
                  SpriteLimiter *limit, bool (*check)(Map, uint8_t, uint8_t)) {
  uint8_t ogcount = limit->current;
  for (uint8_t i = 0; i < retries; i++) {
    if ((limit->current) >= limit->limit || (limit->current) >= ogcount + count)
      break;
    uint8_t x = 1 + RANDB(14);
    uint8_t y = 1 + RANDB(14);
    if (check == NULL || check(gs.map, x, y)) {
      MAPT(gs.map, x, y) = TILERESERVED;
      setup_sprite(graphic, x, y);
      limit->current++;
    }
  }
  return limit->current - ogcount;
}

// TODO: this will need to be a very complex function maybe...
void generate_sprites() {
  reserve_around(gs.map, gs.player.posX, gs.player.posY);
  // Remove any leftover sprites before we add more
  raycast.sprites.resetSprites();
  SpriteLimiter limit;
  limit.limit = NUMOWSPRITES;
  limit.current = 0;
  // These retries and numbers can be tweaked. May store in FX later
  try_spawn(G_ITEMBAG, 4, 1, &limit, has_2x2_box_around);
  try_spawn(G_ROCK, 10, 3, &limit, has_2x2_box_around);
  try_spawn(G_MOUSE, 10, 3, &limit, any_empty);

  // for (uint8_t y = 1; y < gs.map.height - 1; y++) {
  //   for (uint8_t x = 1; x < gs.map.width - 1; x++) {
  //     if (spawn >= NUMSPRITES) // NUMSPRITES)
  //       goto NOMOREGENERATESPRITES;
  //     if (RANDOF(16)) {
  //       if (has_2x2_box_around(gs.map, x, y)) {
  //         MAPT(gs.map, x, y) = TILERESERVED;
  //         RcSprite<NUMINTERNALBYTES> *sp = raycast.sprites.addSprite(
  //             MHALF + x, MHALF + y, ITEMSPRITE, ITEMSIZE, 0, item_hover);
  //         spawn++;
  //         // raycast.sprites
  //       }
  //     }
  //   }
  // }
  // NOMOREGENERATESPRITES:
  remove_reserved_map(gs.map);
}

void check_pickup() {
  // Check for any item sprites overlapping the player and initiate a pickup
  RcSprite<NUMINTERNALBYTES> *sprite =
      sprite_in_tile(gs.player.posX, gs.player.posY);
  if (sprite) {
    uint8_t new_item = sprite->intstate[0];
    if (new_item) {
      if (gs_add_item(&gs, new_item)) {
        raycast.sprites.deleteSprite(sprite);
        draw_items_menu(&gs);
        prep_textarea();
        sg.total_items++;
        tinyfont.print(F("PICKUP: "));
        tinyfont.setCursor(BMESSAGEX + 40, BMESSAGEY);
        print_item_name(new_item);
        item_pickup_beep();
      } else {
        prep_textarea();
        tinyfont.print(F("INVENTORY FULL"));
      }
    }
  }
}

void goto_next_floor() {
  // Restore state from last floor (so generation always the same per seed)
  prng_restore(gen_state);
  gen_region(gs.region);
  // Also generate some items...
  generate_sprites();
  gen_state = prng_snapshot();
  gs.next_player = gs.player;
  // gs.total_floor++;
  gs.region_floor++;
  update_visual_positions(0);
}

// void gen_mymap() {
//   // Dungeon, might be good.
//   //Type2Config c;
//   // c.room_min = 3;
//   // c.room_max = 5;
//   // c.stops = 5;
//   // c.room_unlikely = 1;
//
//   // // Trees, maybe
//   // Type2Config c;
//   // c.room_min = 2;
//   // c.room_max = 3;
//   // c.stops = 15;
//   // c.room_unlikely = 3;
// }
bool is_sprite_solid(RcSprite<NUMINTERNALBYTES> *sprite) {
  return sprite && sprite->frame >= 16;
}

bool movement_solid_sprite_check(GameState *gs, uint8_t x, uint8_t y) {
  return !is_sprite_solid(sprite_in_tile(x, y));
}

bool run_movement(uint8_t movement) {
  bool setstate = false;
  if (movement & (GS_MOVEBACKWARD | GS_MOVEFORWARD)) {
    // Run sim here? Some kind of "action"?
    gs_tickstamina(&gs);
    world_tick();
    step_beep();
    if (gs_dead(&gs)) {
      initiate_gameover();
      setstate = true;
    }
    if (gs_exiting(&gs)) {
      initiate_floor_transition();
      setstate = true;
    }
  }
  if (!setstate) {
    initiate_animation();
  }
  return setstate;
}

void setup() {
  arduboy.boot();
  arduboy.flashlight(); // or safeMode(); for an extra 24 bytes wooo
  arduboy.setFrameRate(FRAMERATE);
  FX_INIT();
  raycast.render.spritescaling[2] = 0.75f;
  raycast.render.spritescaling[3] = 0.6f;
  gs.map.width = RCMAXMAPDIMENSION;
  gs.map.height = RCMAXMAPDIMENSION;
  gs.map.map = raycast.worldMap.map;
  if (!FX::loadGameState(sg)) {
    memset(&sg, 0, sizeof(sg));
  }
  initiate_mainmenu();
}

void loop() {
  if (!(arduboy.nextFrame())) {
    return;
  }
  arduboy.pollButtons();

  uint8_t movement;
  uint24_t raycast_bg = bg;

RESTARTSTATE:;

  switch (gs.state) {
  case GS_STATEMAIN:
#ifdef INSTANTREFRESH
    if (arduboy.justPressed(A_BUTTON)) {
      draw_menu_init();
    }
#endif
#ifdef INSTANTFLOORUP
    if (arduboy.justPressed(A_BUTTON)) {
      initiate_floor_transition();
    }
#endif
    draw_runtime_data(&gs);
    // Check for menu button first
    if (arduboy.justPressed(B_BUTTON)) {
      item_open_beep();
      initiate_itemmenu();
      break;
    }
    movement = gs_move(&gs, &arduboy, movement_solid_sprite_check);
    if (movement) {
      run_movement(movement);
      goto RESTARTSTATE; // Eliminate pause: do animation first frame
    }
    break;
  case GS_STATEANIMATE:
    gs.animframes++;
    update_visual_positions((float)gs.animframes / gs.animend);
    if (gs.animframes >= gs.animend) {
      gs.player = gs.next_player;
      sprites_to_next_position();
      initiate_gamemain();
      check_pickup();
    }
    break;
  case GS_FLOORTRANSITION:
    faze_screen(arduboy.sBuffer);
    gs.animframes++;
    if (gs.animframes >= gs.animend) {
      goto_next_floor();
      initiate_gamemain();
      draw_menu_init();
      prep_textarea();
      if (gs.region == 1 && gs.region_floor == 1) {
        tinyfont.print(F("** SEEK THE TOWER **"));
      } else {
        tinyfont.print(F("ENTERED FLOOR"));
        tinyfont.setCursor(BMESSAGEX + 70, BMESSAGEY);
        tinyfont.print(gs.region_floor);
      }
    }
    goto SKIPRCRENDER;
    break;
  case GS_STATEMENUANIM:
    if (fuzzy_in(titleimg) < 32) {
      gs.state = GS_STATEMENU;
    }
    goto SKIPRCRENDER;
    break;
  case GS_STATEMENU:
    run_menu();
    goto SKIPRCRENDER;
    break;
  case GS_STATEGAMEOVER:
    raycast_bg = 0;
    if (gs.animframes >= gs.animend) {
      arduboy.fillRect(26, 24, 47, 8, BLACK);
      // clear_full_rect(arduboy.sBuffer, {.x = 26, .y = 24, .w = 47, .h = 8});
      tinyfont.setCursor(27, 26);
      tinyfont.print(F("GAME OVER"));
      if (arduboy.justPressed(A_BUTTON)) {
        initiate_mainmenu();
      }
      goto SKIPRCRENDER;
    } else {
      gs.animframes++;
      raycast.render.setLightIntensity(
          2.0 * (1.0 - (float)gs.animframes / gs.animend));
    }
    break;
  case GS_STATEABOUT:
    run_about();
    goto SKIPRCRENDER;
    break;
  case GS_STATEITEMMENU:
    draw_runtime_data(&gs);
    run_itemmenu();
    break;
  case GS_STATEITEMSELECT:
    run_itemselect();
    break;
  }

  // Draw the correct background for the area.
#ifdef RCNOBACKGROUND
  raycast.render.clearRaycast(&arduboy);
#else
  if (raycast_bg) {
    render_fximage(raycast_bg, arduboy.sBuffer, raycast.render.VIEWWIDTH,
                   raycast.render.VIEWHEIGHT);
  }
#endif
  raycast.runIteration(&arduboy);

SKIPRCRENDER:;
  FX::display(false);
}
