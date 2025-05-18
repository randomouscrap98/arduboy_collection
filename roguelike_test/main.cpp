#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Tinyfont.h>

// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

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

constexpr float FOV = 1.0f;
constexpr uint8_t FRAMERATE = 40;
constexpr float MOVESPEED = 4.25f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

constexpr uint8_t DEFAULTANIMFRAMES = ((float)FRAMERATE) * 0.34;

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

void gen_mymap() {
  Type1Config c;
  gen_type_1(&c, gs.map, &gs.player);
  gs.next_player = gs.player;
  update_visual_position(0);
  // IDK where to put this
  render_fximage(menu, arduboy.sBuffer, WIDTH, HEIGHT);
  // FX::drawBitmap(0, 0, menu, 0, dbmOverwrite);
  gs_draw_map(&gs, &arduboy, WIDTH - SIDESIZE + 4, 20);
}

// constexpr uint8_t MENUMAX = 6;
// // clang-format off
// constexpr char MENULETTER[][MENUMAX] = {
//   "MINWD",
//   "DRBUF",
//   "MXWRT",
//   "9POOL",
//   "CBCPL",
//   "CBMXL",
//   "BMPPL",
//  //  "M",
//  //  "D",
//  //  "M",
//  //  "9",
//  //  "C",
//  //  "C",
//  //  "B",
// };
// // clang-format on
// constexpr uint8_t MENUITEMS = sizeof(MENULETTER) / MENUMAX;
//
// uint8_t menu_pos = 0;
// void do_menu() {
//   uint8_t *mval;
//   char line[MENUMAX + 10];
//   for (int i = 0; i < MENUITEMS; i++) {
//     tinyfont.setCursor(64, i * 5);
//     tinyfont.print(i == menu_pos ? '>' : ' ');
//     tinyfont.print(MENULETTER[i]);
//     tinyfont.print(' ');
//     uint8_t *val;
//     switch (i) {
//     case 0:
//       val = &config.minwidth;
//       break;
//     case 1:
//       val = &config.doorbuffer;
//       break;
//     case 2:
//       val = &config.maxwallretries;
//       break;
//     case 3:
//       val = &config.ninepool;
//       break;
//     case 4:
//       val = &config.cubiclepool;
//       break;
//     case 5:
//       val = &config.cubiclemaxlength;
//       break;
//     case 6:
//       val = &config.bumppool;
//       break;
//     }
//     tinyfont.print(*val);
//     if (i == menu_pos) {
//       mval = val;
//     }
//   }
//   int8_t dir = 0;
//   if (arduboy.justPressed(DOWN_BUTTON))
//     dir = 1;
//   if (arduboy.justPressed(UP_BUTTON))
//     dir = -1;
//   menu_pos = (menu_pos + dir + MENUITEMS) % MENUITEMS;
//   if (arduboy.justPressed(RIGHT_BUTTON))
//     (*mval) += 1;
//   if (arduboy.justPressed(LEFT_BUTTON))
//     (*mval) -= 1;
//   if (arduboy.justPressed(A_BUTTON)) {
//     gen_mymap();
//     sound.tone(300, 30);
//   }
// }

void setup() {
  arduboy.boot();
  arduboy.flashlight();
  arduboy.setFrameRate(FRAMERATE);
  arduboy.initRandomSeed();
  FX_INIT();
  // NOTE: second value FOV
  // raycast.player.initPlayerDirection(0, 1.0); // 0.75);
  raycast.render.setLightIntensity(2.0);
  raycast.render.spritescaling[2] = 0.75;
  raycast.render.spriteShading = RcShadingType::Black;
  gs.map.width = RCMAXMAPDIMENSION;
  gs.map.height = RCMAXMAPDIMENSION;
  gs.map.map = raycast.worldMap.map;
  gs.state = GS_STATEMAIN;
  gs.animend = DEFAULTANIMFRAMES;
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
    if (arduboy.justPressed(A_BUTTON)) {
      gen_mymap();
      sound.tone(300, 30);
    }
    if (gs_move(&gs, &arduboy)) {
      gs.animframes = 0; // begin animation at 0 (?)
      gs.state = GS_STATEANIMATE;
      goto RESTARTSTATE; // Eliminate pause: do animation first frame
    }
    break;
  case GS_STATEANIMATE:
    gs.animframes++;
    update_visual_position((float)gs.animframes / (float)gs.animend);
    if (gs.animframes >= gs.animend) {
      gs.state = GS_STATEMAIN;
      gs.player = gs.next_player;
    }
    break;
  }

  // Draw the correct background for the area.
  // raycast.render.drawRaycastBackground(&arduboy, bg_full);
  render_fximage(bg, arduboy.sBuffer, raycast.render.VIEWWIDTH,
                 raycast.render.VIEWHEIGHT);
  // FX::drawBitmap(0, 0, bg, 0, dbmOverwrite);
  raycast.runIteration(&arduboy);

  FX::display(false);
}
