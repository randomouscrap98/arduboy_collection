#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Tinyfont.h>

// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

// #define FULLMAP

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

constexpr uint8_t NUMINTERNALBYTES = 1;
constexpr uint8_t NUMSPRITES = 16;
constexpr uint8_t BOTTOMSIZE = 8;
constexpr uint8_t SIDESIZE = 32;
constexpr uint8_t MAPX = WIDTH - 18;
constexpr uint8_t MAPY = 2;
constexpr uint8_t MAPRANGE = 1;
constexpr uint8_t MAPFLASH = 8;

constexpr uint16_t TINYDIGITS[] = {0xF9F, 0xAF8, 0xDDA, 0x9DF, 0x74F,
                                   0xBDD, 0xFDC, 0x11F, 0xFCF, 0x75F};

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
    uint8_t dig = (TINYDIGITS[v] >> ((2 - i) * 4)) & 0xF;
    arduboy.sBuffer[x + i + (y >> 3) * WIDTH] |= (dig << (y & 7));
  }
}

// uint8_t lcg8(uint8_t x) { return (0x41 * x + 0x1F); }
uint8_t prng() {
  // Seed this 8bit manually
  static uint8_t s = 0xAA, a = 0;
  s ^= s << 3;
  s ^= s >> 5;
  s ^= a++ >> 2;
  return s;
}

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
  c.room_min = 3;
  // c.room_max = 5;
  c.stops = 5;
  c.room_unlikely = 1;
  c.tiles.main = 1;
  c.tiles.perimeter = 7;
  c.tiles.exit = 15;

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
  print_tinydigit(gs.region_floor / 10, 120, 20);
  print_tinydigit(gs.region_floor % 10, 124, 20);
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

void rcd() {
  // Draw the correct background for the area.
  render_fximage(bg, arduboy.sBuffer, raycast.render.VIEWWIDTH,
                 raycast.render.VIEWHEIGHT);
  raycast.runIteration(&arduboy);
}

void setup() {
  arduboy.boot();
  arduboy.flashlight();
  arduboy.setFrameRate(FRAMERATE);
  arduboy.initRandomSeed();
  FX_INIT();
  // NOTE: second value FOV
  // raycast.player.initPlayerDirection(0, 1.0); // 0.75);
  raycast.render.setLightIntensity(1.5); // 2.0
  raycast.render.spritescaling[2] = 0.75;
  raycast.render.spriteShading = RcShadingType::Black;
  gs.map.width = RCMAXMAPDIMENSION;
  gs.map.height = RCMAXMAPDIMENSION;
  gs.map.map = raycast.worldMap.map;
  gs.state = GS_STATEMAIN;
  gs.animend = DEFAULTANIMFRAMES;
  gs.region = 1;
  gs.region_floor = 0;
  gs.total_floor = 0;
  gen_mymap();
}

void loop() {
  if (!(arduboy.nextFrame())) {
    return;
  }
  arduboy.pollButtons();

RESTARTSTATE:;

  switch (gs.state) {
  case GS_STATEMAIN:
    // if (arduboy.justPressed(A_BUTTON)) {
    // }
    gs_draw_map_player(&gs, &arduboy, MAPX, MAPY,
                       arduboy.frameCount & MAPFLASH ? BLACK : WHITE);
    if (gs_move(&gs, &arduboy)) {
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
  render_fximage(bg, arduboy.sBuffer, raycast.render.VIEWWIDTH,
                 raycast.render.VIEWHEIGHT);
  raycast.runIteration(&arduboy);

SKIPRCRENDER:;
  FX::display(false);
}
