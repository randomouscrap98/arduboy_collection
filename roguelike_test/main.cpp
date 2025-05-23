#include "Arduboy2Core.h"
#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Tinyfont.h>
#include <avr/pgmspace.h>

// This saves ~2700 pgm and 110 ram, will need exitToBootloader in menu
// ARDUBOY_NO_USB

// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

#define FULLMAP
#define INSTANTFLOORUP
// #define PRINTSTAMHEALTH
#define PRINTSEED

#include <ArduboyFX.h>
#include <ArduboyRaycastFX.h>

#include "ArduboyRaycast_Map.h"
#include "game.hpp"
#include "map.hpp"

// Resources
#include "fxdata/fxdata.h"

Arduboy2Base arduboy;

ArduboyTones sound(arduboy.audio.enabled);
Tinyfont tinyfont =
    Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint16_t TINYDIGITS[] PROGMEM = {
    0xF9F, 0xAF8, 0xDDA, 0x9DF, 0x74F, 0xBDD, 0xFDC, 0x11F, 0xFDF, 0x75F,
};

constexpr uint8_t NUMINTERNALBYTES = 8;
constexpr uint8_t NUMSPRITES = 16;
constexpr uint8_t BOTTOMSIZE = 8;
constexpr uint8_t SIDESIZE = 32;
constexpr uint8_t MAPX = WIDTH - 18;
constexpr uint8_t MAPY = 2;
constexpr uint8_t MAPRANGE = 1;
constexpr uint8_t MAPFLASH = 8;
constexpr uint8_t BARWIDTH = 2;
constexpr uint8_t BARHEIGHT = 16;
constexpr uint8_t BARTOP = 2;
constexpr uint8_t HEALTHBARX = 99;
constexpr uint8_t STAMINABARX = 104;
constexpr uint8_t BASESTAMHEALTH = 255;

constexpr float FOV = 1.0f;
constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 4.25f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

constexpr uint8_t DEFAULTANIMFRAMES = ((float)FRAMERATE) * 0.34;
constexpr uint8_t DEFAULTANIMEXITFRAMES = ((float)FRAMERATE) * 0.6;

// Once again we pick 16 sprites just in case we need them. 16 is a decent
// number to not take up all the memory but still have enough to work with.
RcContainer<NUMSPRITES, NUMINTERNALBYTES, WIDTH - SIDESIZE, HEIGHT - BOTTOMSIZE>
    raycast(tilesheet, NULL, NULL); // spritesheet, spritesheetMask);

GameState gs;
SaveGame sg;

void initiate_floor_transition() {
  gs.animframes = 0; // begin animation at 0 (?)
  gs.state = GS_FLOORTRANSITION;
  gs.animend = DEFAULTANIMEXITFRAMES;
}

void begin_game() {
  if (sg.player_seed == 0) { // Sorry, the 0 seed is reserved I guess...
    sg.player_seed = (uint16_t)arduboy.generateRandomSeed();
  }
  sg.total_runs++;
  FX::saveGameState(sg);
  gs.region = 1;
  gs.region_floor = 0;
  gs.total_floor = 0;
  gs.stamina = BASESTAMHEALTH;
  gs.health = BASESTAMHEALTH;
  gs.menu_pos = 0;
  // TODO: will need to generate a player seed based on what... frame count(?)
  // and have iteration be their current run count. Will generate save and
  // update values on new game, so the frame count is a bit more random.
  // Iteration will start at 0, then increment by 1 on NEW GAME so people can't
  // save scum(?)
  prng_seed(lcg_shuffle(sg.player_seed, sg.total_runs)); // change this later
  initiate_floor_transition();
}

// Update animation rotation/position based on given delta between current
// position and new player position. Will NOT be extended to enemies, since
// they don't have rotation and their position is different
void update_visual_position(float delta) {
  raycast.player.posX = 0.5f + ((float)gs.player.posX * (1 - delta) +
                                (float)gs.next_player.posX * delta);
  raycast.player.posY = 0.5f + ((float)gs.player.posY * (1 - delta) +
                                (float)gs.next_player.posY * delta);
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
  raycast.player.initPlayerDirection((1 - delta) * c1 + delta * c2, FOV);
}

// very optimized render image at specific address byte chunks only
void render_fximage(uint24_t addr, uint8_t *at, uint8_t width, uint8_t height) {
  height = height >> 3; // it's actually per byte ofc
  for (uint8_t i = 0; i < height; i++) {
    FX::readDataBytes(addr + 4 + i * width, at + i * WIDTH, width);
  }
}

// Print tiny digits at given location
void print_tinydigit(uint8_t v, uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < 3; i++) {
    uint8_t dig = ((pgm_read_word(TINYDIGITS + v)) >> ((2 - i) * 4)) & 0xF;
    arduboy.sBuffer[x + i + (y >> 3) * WIDTH] |= (dig << (y & 7));
  }
}

// Print a number to the given amount of digits
void print_tinynumber(uint16_t v, uint8_t w, uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < w; i++) {
    print_tinydigit(v % 10, x + 4 * (w - i - 1), y);
    v /= 10;
  }
}

void faze_screen() {
  for (uint16_t i = 0; i < 1024; i++) {
    arduboy.sBuffer[i] <<= prng() & 3;
  }
}

void clear_full_rect(MRect r) {
  r.y >>= 3;
  r.h >>= 3;
  for (uint8_t y = 0; y < r.h; y++) {
    for (uint8_t x = 0; x < r.w; x++) {
      arduboy.sBuffer[x + r.x + (y + r.y) * WIDTH] = 0;
    }
  }
}

uint16_t menu_animation() {
  uint16_t pending = 0;
  uint8_t b;
  for (uint16_t i = 0; i < 1024; i++) {
    FX::readDataBytes(titleimg + 4 + i, &b, 1);
    if (arduboy.sBuffer[i] == b)
      continue;
    arduboy.sBuffer[i] = b << (prng() & 7);
    pending++;
  }
  return pending;
}

void run_menu() {
  constexpr uint8_t MENUOPTIONS = 4;
  // clear_full_rect({.x = 7, .y = 8, .w = 50, .h = 24});
  render_fximage(titleimg, arduboy.sBuffer, WIDTH, HEIGHT);
  tinyfont.setCursor(12, 12);
  tinyfont.print(F("NEW GAME"));
  tinyfont.setCursor(12, 18);
  tinyfont.print(arduboy.audio.enabled() ? F("SOUND:ON") : F("SOUND:OFF"));
  tinyfont.setCursor(12, 24);
  tinyfont.print(F("ABOUT"));
  tinyfont.setCursor(12, 30);
  tinyfont.print(F("QUIT"));
  gs.menu_pos =
      (gs.menu_pos + MENUOPTIONS + (arduboy.justPressed(DOWN_BUTTON) ? 1 : 0) -
       (arduboy.justPressed(UP_BUTTON) ? 1 : 0)) %
      MENUOPTIONS;
  tinyfont.setCursor(7, 12 + 6 * gs.menu_pos);
  tinyfont.print(F("#"));
  if (arduboy.justPressed(A_BUTTON)) {
    switch (gs.menu_pos) {
    case 0:
      // Need to do a BIT more...
      begin_game();
      break;
    case 1:
      // Don't save it (for now)
      arduboy.audio.toggle();
      break;
    case 3:
      // Can exit immediately
      arduboy.audio.saveOnOff(); // Might as well
      arduboy.exitToBootloader();
      break;
    }
  }
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
    c.room_min = 2;
    c.stops = 10;
    c.room_unlikely = 3;
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
void refresh_screen_full() {
  render_fximage(menu, arduboy.sBuffer, WIDTH, HEIGHT);
  print_tinydigit(gs.region, 113, 20);
  print_tinynumber(gs.region_floor, 2, 120, 20);
#ifdef FULLMAP
  gs_draw_map(&gs, &arduboy, MAPX, MAPY);
#else
  gs_draw_map_near(&gs, &arduboy, MAPX, MAPY, MAPRANGE);
#endif
}

void goto_next_floor() {
  gen_region(gs.region);
  gs.next_player = gs.player;
  gs.total_floor++;
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

// ASSUMING THE TEXT AREA DECORATION IS KNOWN, this will very VERY quickly clear
// JUST the text area fully. It's very fast and very little code...
void clear_textarea() {
  memset(arduboy.sBuffer + (WIDTH * ((HEIGHT >> 3) - 1)), 1, WIDTH);
}

// Draw a standard 2x16 bar at given x, y chosen by default
void draw_std_bar(uint8_t x, uint8_t filled, uint8_t max) {
  float ffilled = BARHEIGHT * (float)filled / (float)max;
  uint8_t fh = BARHEIGHT - round(ffilled);
  for (uint8_t y = 0; y < BARHEIGHT; y++) {
    uint8_t col = y < fh ? BLACK : WHITE;
    arduboy.drawPixel(x, BARTOP + y, col);
    arduboy.drawPixel(x + 1, BARTOP + y, col);
  }
}

void draw_runtime_data() {
  draw_std_bar(HEALTHBARX, gs.health, BASESTAMHEALTH);
  draw_std_bar(STAMINABARX, gs.stamina, BASESTAMHEALTH);
#ifdef PRINTSTAMHEALTH
  clear_textarea();
  print_tinynumber(gs.health, 3, 0, HEIGHT - 5);
  print_tinynumber(gs.stamina, 3, 16, HEIGHT - 5);
#endif
#ifdef PRINTSEED
  clear_textarea();
  print_tinynumber(sg.player_seed, 5, 0, HEIGHT - 5);
  print_tinynumber(sg.total_runs, 5, 24, HEIGHT - 5);
#endif
  gs_draw_map_player(&gs, &arduboy, MAPX, MAPY,
                     arduboy.frameCount & MAPFLASH ? BLACK : WHITE);
}

void runAction() { gs_tickstamina(&gs); }

void setup() {
  arduboy.boot();
  arduboy.flashlight(); // or safeMode(); for an extra 24 bytes wooo
  arduboy.setFrameRate(FRAMERATE);
  FX::begin(FX_DATA_PAGE, FX_SAVE_PAGE);
  // NOTE: second value FOV
  // raycast.player.initPlayerDirection(0, 1.0); // 0.75);
  raycast.render.spritescaling[2] = 0.75;
  gs.map.width = RCMAXMAPDIMENSION;
  gs.map.height = RCMAXMAPDIMENSION;
  gs.map.map = raycast.worldMap.map;
  gs.animend = DEFAULTANIMFRAMES;
  gs.state = GS_STATEMENUANIM;
  if (!FX::loadGameState(sg)) {
    memset(&sg, 0, sizeof(sg));
  }
  // begin_game();
}

void loop() {
  if (!(arduboy.nextFrame())) {
    return;
  }
  arduboy.pollButtons();

  uint8_t movement;
RESTARTSTATE:;

  switch (gs.state) {
  case GS_STATEMAIN:
#ifdef INSTANTFLOORUP
    if (arduboy.justPressed(A_BUTTON)) {
      initiate_floor_transition();
    }
#endif
    draw_runtime_data();
    movement = gs_move(&gs, &arduboy);
    if (movement) {
      // Run sim here? Some kind of "action"?
      if (movement & (GS_MOVEBACKWARD | GS_MOVEFORWARD)) {
        runAction();
      }
      if (gs_exiting(&gs)) {
        initiate_floor_transition();
      } else {
        gs.animframes = 0; // begin animation at 0 (?)
        gs.state = GS_STATEANIMATE;
        gs.animend = DEFAULTANIMFRAMES;
      }
      goto RESTARTSTATE; // Eliminate pause: do animation first frame
    }
    break;
  case GS_STATEANIMATE:
    gs.animframes++;
    update_visual_position((float)gs.animframes / (float)gs.animend);
    if (gs.animframes >= gs.animend) {
      gs.state = GS_STATEMAIN;
      gs.player = gs.next_player;
      gs_draw_map_near(&gs, &arduboy, MAPX, MAPY, MAPRANGE);
    }
    break;
  case GS_FLOORTRANSITION:
    faze_screen();
    gs.animframes++;
    if (gs.animframes >= gs.animend) {
      gs.state = GS_STATEMAIN;
      goto_next_floor();
      refresh_screen_full();
      sound.tone(300, 30);
    }
    goto SKIPRCRENDER;
    break;
  case GS_STATEMENUANIM:
    if (menu_animation() < 32) {
      gs.state = GS_STATEMENU;
    }
    goto SKIPRCRENDER;
    break;
  case GS_STATEMENU:
    run_menu();
    goto SKIPRCRENDER;
    break;
  }

  // Draw the correct background for the area.
  render_fximage(bg, arduboy.sBuffer, raycast.render.VIEWWIDTH,
                 raycast.render.VIEWHEIGHT);
  raycast.runIteration(&arduboy);

SKIPRCRENDER:;
  FX::display(false);
}
