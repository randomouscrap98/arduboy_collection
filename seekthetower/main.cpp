// This saves 1000 bytes but doesn't help the overdraw glitch
// #define RCSMALLLOOPS

#include <Arduboy2.h>
#include <Tinyfont.h>
#include <avr/pgmspace.h>

// This saves ~2700 pgm and 110 ram, will need exitToBootloader in menu
// ARDUBOY_NO_USB

// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

// #define FULLMAP
// #define INSTANTFLOORUP
// #define INSTANTREFRESH
// #define PRINTSTAMHEALTH
// #define PRINTSEED
// #define PRINTDIRXY
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
constexpr uint8_t MAPX = WIDTH - 18;
constexpr uint8_t MAPY = 2;
constexpr uint8_t MAPRANGE = 1;
constexpr uint8_t MAPFLASH = 8;
constexpr uint8_t HEALTHBARX = 99;
constexpr uint8_t STAMINABARX = 104;

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

// Update animation rotation/position based on given delta between current
// position and new player position. Will NOT be extended to enemies, since
// they don't have rotation and their position is different
void update_visual_position(uflot delta) {
  uflot indelt = 1 - delta;
  uflot baseX = HALF + gs.player.posX * (indelt);
  uflot baseY = HALF + gs.player.posY * (indelt);
  raycast.player.posX = baseX + delta * gs.next_player.posX;
  raycast.player.posY = baseY + delta * gs.next_player.posY;
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
  clear_items_menu();
  if (arduboy.justPressed(A_BUTTON) && gs_has_item(&gs, gs.item_pos)) {
    menu_select_beep();
    gs.tempstate1 = 1; // Don't let b cancel the whole menu
    gs.menu_pos = 0;
    initiate_itemselect();
  }
  if (arduboy.justReleased(B_BUTTON)) {
    if (gs.tempstate1) {
      // menu_select_beep();
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

struct SpriteDefinition {
  uint8_t sprite;
  uint8_t size;
  uint8_t offset;
  void (*func)(RcSprite<NUMINTERNALBYTES> *);
};

constexpr SpriteDefinition sprite_definitions[] PROGMEM = {
    {
        1, // Item bag
        2,
        0,
        item_hover,
    },
    {
        2, // rocks
        3,
        4,
        NULL,
    },
};

uint8_t try_place_2x2(uint8_t item, uint8_t retries, uint8_t count) {
  uint8_t placed = 0;
  for (uint8_t i = 0; i < retries; i++) {
    uint8_t x = 1 + RANDB(14);
    uint8_t y = 1 + RANDB(14);
    if (has_2x2_box_around(gs.map, x, y)) {
      MAPT(gs.map, x, y) = TILERESERVED;
      SpriteDefinition sp;
      memcpy_P(&sp, sprite_definitions + item, sizeof(SpriteDefinition));
      // RcSprite<NUMINTERNALBYTES> *sp =
      raycast.sprites.addSprite(MHALF + x, MHALF + y, sp.sprite, sp.size,
                                sp.offset, sp.func);
      placed++;
      if (placed >= count)
        break;
    }
  }
  return placed;
}

// TODO: this will need to be a very complex function maybe...
void generate_sprites() {
  // Remove any leftover sprites before we add more
  raycast.sprites.resetSprites();
  uint8_t spawn = 0;
  // These retries and numbers can be tweaked. May store in FX later
  spawn += try_place_2x2(0, 4, 1);
  if (spawn >= NUMSPRITES)
    goto NOMOREGENERATESPRITES;
  spawn += try_place_2x2(1, 10, 3);
  if (spawn >= NUMSPRITES)
    goto NOMOREGENERATESPRITES;

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
NOMOREGENERATESPRITES:
  remove_reserved_map(gs.map);
}

void check_pickup() {
  // Check for any item sprites overlapping the player and initiate a pickup
  for (uint8_t i = 0; i < NUMSPRITES; i++) {
    RcSprite<NUMINTERNALBYTES> *sprite = &raycast.sprites.sprites[i];
    if (!ISSPRITEACTIVE((*sprite)))
      continue;
    if (sprite->x.getInteger() == gs.player.posX &&
        sprite->y.getInteger() == gs.player.posY) {
      uint8_t new_item = 0;
      switch (sprite->frame) {
      case 1:
        if (RANDOF(12)) {
          new_item = ITEM_REVIVE; // Revive spirit
        } else {
          new_item = RANDOF(3) ? ITEM_POTION
                               : ITEM_FOOD; // one in three chance for potion
        }
        break;
      case 2:
        new_item = ITEM_ROCK; // rock
        break;
      }
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
          break;
        } else {
          prep_textarea();
          tinyfont.print(F("INVENTORY FULL"));
        }
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
  update_visual_position(0);
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

void draw_runtime_data() {
  draw_std_bar(HEALTHBARX, gs.health, BASESTAMHEALTH);
  draw_std_bar(STAMINABARX, gs.stamina, BASESTAMHEALTH);
  draw_map_near(&gs, MAPX, MAPY, MAPRANGE);
  draw_map_player(&gs, MAPX, MAPY,
                  arduboy.frameCount & MAPFLASH ? BLACK : WHITE);
#ifdef PRINTDIRXY
  prep_textarea();
  tinyfont.setCursor(0, HEIGHT - 6);
  tinyfont.print(raycast.player.dirX);
  tinyfont.setCursor(30, HEIGHT - 6);
  tinyfont.print(raycast.player.dirY);
#endif
#ifdef PRINTSTAMHEALTH
  prep_textarea();
  print_tinynumber(arduboy.sBuffer, gs.health, 3, 0, HEIGHT - 5);
  print_tinynumber(arduboy.sBuffer, gs.stamina, 3, 16, HEIGHT - 5);
#endif
#ifdef PRINTSEED
  prep_textarea();
  print_tinynumber(arduboy.sBuffer, sg.player_seed, 5, 0, HEIGHT - 5);
  print_tinynumber(arduboy.sBuffer, sg.total_runs, 5, 24, HEIGHT - 5);
#endif
}

bool run_movement(uint8_t movement) {
  bool setstate = false;
  if (movement & (GS_MOVEBACKWARD | GS_MOVEFORWARD)) {
    // Run sim here? Some kind of "action"?
    gs_tickstamina(&gs);
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
    draw_runtime_data();
    // Check for menu button first
    if (arduboy.justPressed(B_BUTTON)) {
      item_open_beep();
      initiate_itemmenu();
      break;
    }
    movement = gs_move(&gs, &arduboy);
    if (movement) {
      run_movement(movement);
      goto RESTARTSTATE; // Eliminate pause: do animation first frame
    }
    break;
  case GS_STATEANIMATE:
    gs.animframes++;
    update_visual_position((uflot)gs.animframes / gs.animend);
    if (gs.animframes >= gs.animend) {
      gs.player = gs.next_player;
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
    draw_runtime_data();
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
