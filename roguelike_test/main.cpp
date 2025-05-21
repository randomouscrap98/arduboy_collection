#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Tinyfont.h>
#include <avr/pgmspace.h>

// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

// #define FULLMAP
// #define INSTANTFLOORUP

#include <ArduboyFX.h>
#include <ArduboyRaycastFX.h>

#include "ArduboyRaycast_Map.h"
#include "game.hpp"
#include "map.hpp"

// Resources
// #include "bg_full.h"
#include "fxdata/fxdata.h"

Arduboy2 arduboy;
bool always_on() { return true; }

ArduboyTones sound(&always_on); // arduboy.audio.enabled);
// RoomConfig config;
Tinyfont tinyfont =
    Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint16_t TINYDIGITS[] PROGMEM = {
    0xF9F, 0xAF8, 0xDDA, 0x9DF, 0x74F, 0xBDD, 0xFDC, 0x11F, 0xFDF, 0x75F,
};

constexpr uint8_t NUMINTERNALBYTES = 1;
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

// uint8_t lcg8(uint8_t x) { return (0x41 * x + 0x1F); }
// uint8_t prng() {
//   // Seed this 8bit manually
//   static uint8_t s = 0xAA, a = 0;
//   s ^= s << 3;
//   s ^= s >> 5;
//   s ^= a++ >> 2;
//   return s;
// }

void faze_screen() {
  for (uint16_t i = 0; i < 1024; i++) {
    // if (lcg8(arduboy.frameCount + lcg8(i)) & 1)
    arduboy.sBuffer[i] <<= prng() & 3;
    // if (prng() & 1)
    //   arduboy.sBuffer[i] <<= 2;
    // else
    //   arduboy.sBuffer[i] >>= 2;
  }
}

void gen_mymap() {
  // Dungeon, might be good.
  Type2Config c;
  // c.room_min = 3;
  // c.room_max = 5;
  // c.stops = 5;
  // c.room_unlikely = 1;
  c.room_min = 2;
  c.stops = 10;
  c.room_unlikely = 3;
  c.tiles.main = 1;
  c.tiles.perimeter = 7;
  c.tiles.exit = 15;
  c.tiles.extras_count = 2;
  c.tiles.extras[0].tile = 2;
  c.tiles.extras[0].type = TILEEXTRATYPE_NORMAL;
  c.tiles.extras[0].unlikely = 10;
  c.tiles.extras[1].tile = 3;
  c.tiles.extras[1].type = TILEEXTRATYPE_NOCORNER;
  c.tiles.extras[1].unlikely = 2;

  // // Trees, maybe
  // Type2Config c;
  // c.room_min = 2;
  // c.room_max = 3;
  // c.stops = 15;
  // c.room_unlikely = 3;
  gen_type_2(&c, gs.map, &gs.player);
  gs.next_player = gs.player;
  gs.total_floor++;
  gs.region_floor++;
  update_visual_position(0);
  render_fximage(menu, arduboy.sBuffer, WIDTH, HEIGHT);
  print_tinydigit(gs.region, 113, 20);
  print_tinynumber(gs.region_floor, 2, 120, 20);
  // print_tinydigit(gs.region_floor / 10, 120, 20);
  // print_tinydigit(gs.region_floor % 10, 124, 20);
  // tinyfont.setCursor(113, 20);
  // tinyfont.print(gs.region);
  //  tinyfont.setCursor(120, 20);
  //  if (gs.region_floor < 10) {
  //    tinyfont.print(0);
  //  }
  //  tinyfont.print(gs.region_floor);
#ifdef FULLMAP
  gs_draw_map(&gs, &arduboy, MAPX, MAPY);
#else
  gs_draw_map_near(&gs, &arduboy, MAPX, MAPY, MAPRANGE);
#endif
}

// void rcd() {
//   // Draw the correct background for the area.
//   render_fximage(bg, arduboy.sBuffer, raycast.render.VIEWWIDTH,
//                  raycast.render.VIEWHEIGHT);
//   raycast.runIteration(&arduboy);
// }

void clear_textarea() {
  memset(arduboy.sBuffer + (WIDTH * ((HEIGHT >> 3) - 1)), 1, WIDTH);
}

// Draw a standard 2x16 bar at given x, y chosen by default
void draw_std_bar(uint8_t x, uint8_t filled, uint8_t max) {
  // uint8_t bs = 256 / a.h; // let's hope this is SOMETHING...
  // uint8_t fh = filled / bs + 1 - a.h;
  // for (uint8_t y = 0; y < a.h; y++) {
  //   uint8_t col = y < fh ? BLACK : WHITE;
  //   for (uint8_t x = 0; x < a.w; x++) {
  //     arduboy.drawPixel(a.x + x, a.y + y, col);
  //   }
  // }
  // uint8_t bs = 256 / a.h; // let's hope this is SOMETHING...
  float ffilled = BARHEIGHT * (float)filled / (float)max;
  uint8_t fh = BARHEIGHT - round(ffilled);
  // BARHEIGHT - filled / (256 / BARHEIGHT);
  for (uint8_t y = 0; y < BARHEIGHT; y++) {
    uint8_t col = y < fh ? BLACK : WHITE;
    arduboy.drawPixel(x, BARTOP + y, col);
    arduboy.drawPixel(x + 1, BARTOP + y, col);
  }
}

void runAction() { gs_tickstamina(&gs); }

void resetGame() {
  gs.map.width = RCMAXMAPDIMENSION;
  gs.map.height = RCMAXMAPDIMENSION;
  gs.map.map = raycast.worldMap.map;
  gs.state = GS_STATEMAIN;
  gs.animend = DEFAULTANIMFRAMES;
  gs.region = 1;
  gs.region_floor = 0;
  gs.total_floor = 0;
  gs.stamina = BASESTAMHEALTH;
  gs.health = BASESTAMHEALTH;
}

void setup() {
  arduboy.boot();
  arduboy.flashlight();
  arduboy.setFrameRate(FRAMERATE);
  // arduboy.initRandomSeed();
  FX_INIT();
  // NOTE: second value FOV
  // raycast.player.initPlayerDirection(0, 1.0); // 0.75);
  raycast.render.setLightIntensity(1.5); // 2.0
  raycast.render.spritescaling[2] = 0.75;
  raycast.render.spriteShading = RcShadingType::Black;
  resetGame();
  gen_mymap();
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
      gs.animframes = 0; // begin animation at 0 (?)
      gs.state = GS_FLOORTRANSITION;
      gs.animend = DEFAULTANIMEXITFRAMES;
    }
#endif
    draw_std_bar(HEALTHBARX, gs.health, BASESTAMHEALTH);
    draw_std_bar(STAMINABARX, gs.stamina, BASESTAMHEALTH);
    clear_textarea();
    print_tinynumber(gs.health, 3, 0, HEIGHT - 5);
    print_tinynumber(gs.stamina, 3, 16, HEIGHT - 5);
    gs_draw_map_player(&gs, &arduboy, MAPX, MAPY,
                       arduboy.frameCount & MAPFLASH ? BLACK : WHITE);
    movement = gs_move(&gs, &arduboy);
    if (movement) {
      // Run sim here? Some kind of "action"?
      if (movement & (GS_MOVEBACKWARD | GS_MOVEFORWARD)) {
        runAction();
      }
      gs.animframes = 0; // begin animation at 0 (?)
      if (gs_exiting(&gs)) {
        gs.state = GS_FLOORTRANSITION;
        gs.animend = DEFAULTANIMEXITFRAMES;
      } else {
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
      gen_mymap();
      sound.tone(300, 30);
    }
    goto SKIPRCRENDER;
    break;
  }

  // Draw the correct background for the area.
  // draw_v_bar({.x = 99, .y = 2, .w = 2, .h = 16}, gs.health);
  // draw_v_bar({.x = 104, .y = 2, .w = 2, .h = 16}, gs.stamina);
  //  draw_v_bar({.x = 108, .y = 2, .w = 2, .h = 16}, gs.stamina);
  //  draw_v_bar({.x = 110, .y = 2, .w = 2, .h = 16}, gs.stamina);
  render_fximage(bg, arduboy.sBuffer, raycast.render.VIEWWIDTH,
                 raycast.render.VIEWHEIGHT);
  raycast.runIteration(&arduboy);

SKIPRCRENDER:;
  FX::display(false);
}
