#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Tinyfont.h>

// JUST TO SHUTUP CLANGD
// #ifndef __uint24
// #define __uint24 uint32_t
// #endif

#include <ArduboyFX.h>
#include <ArduboyRaycastFX.h>

// #include "cope.hpp"

// #include "Arduboy2Core.h"
#include "ArduboyRaycast_Map.h"
#include "gamestate.hpp"
#include "map.hpp"

// Resources
#include "bg_full.h"
#include "fxdata/fxdata.h"

Arduboy2 arduboy;
bool always_on() { return true; }

ArduboyTones sound(&always_on); // arduboy.audio.enabled);
// RoomConfig config;
Tinyfont tinyfont =
    Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

constexpr uint8_t NUMINTERNALBYTES = 1;
constexpr uint8_t NUMSPRITES = 16;

constexpr float FOV = 1.0f;
constexpr uint8_t FRAMERATE = 30;
constexpr float MOVESPEED = 4.25f / FRAMERATE;
constexpr float ROTSPEED = 3.0f / FRAMERATE;

// Once again we pick 16 sprites just in case we need them. 16 is a decent
// number to not take up all the memory but still have enough to work with.
RcContainer<NUMSPRITES, NUMINTERNALBYTES, WIDTH - 36, HEIGHT - 8>
    raycast(tilesheet, spritesheet, spritesheetMask);

GameState gs;

// bool isSolid(uflot x, uflot y) {
//   // The location is solid if the map cell is nonzero OR if we're colliding
//   with
//   // any (solid) bounding boxes
//   uint8_t tile = raycast.worldMap.getCell(x.getInteger(), y.getInteger());
//
//   return (tile != 0) ||
//          raycast.sprites.firstColliding(x, y, RBSTATESOLID) != NULL;
// }

// void movement() {
//   float movement = 0;
//   float rotation = 0;
//
//   // Simple movement forward and backward. Might as well also let the user
//   //use B
//   // to move forward for "tank" controls
//   if (arduboy.pressed(UP_BUTTON) || arduboy.pressed(B_BUTTON))
//     movement = MOVESPEED;
//   if (arduboy.pressed(DOWN_BUTTON))
//     movement = -MOVESPEED;
//
//   // Simple rotation
//   if (arduboy.pressed(RIGHT_BUTTON))
//     rotation = -ROTSPEED;
//   if (arduboy.pressed(LEFT_BUTTON))
//     rotation = ROTSPEED;
//
//   raycast.player.tryMovement(movement, rotation, &isSolid);
// }

void gen_mymap() {
  Type1Config c;
  gen_type_1(&c, gs.map, &gs.player);
  raycast.player.posX = 0.5f + (float)gs.player.posX;
  raycast.player.posY = 0.5f + (float)gs.player.posY;
  raycast.player.initPlayerDirection(MPLAYRAD(gs.player), FOV);
  arduboy.fillRect(100, 20, 28, 44, BLACK);
  tinyfont.setCursor(100, 20);
  tinyfont.print(gs.player.dirX);
  tinyfont.print(',');
  tinyfont.print(gs.player.dirY);
  gs_draw_map(&gs, &arduboy, WIDTH - 16, 0);
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
  gen_mymap();
}

void loop() {
  if (!(arduboy.nextFrame())) {
    return;
  }
  arduboy.pollButtons();
  if (arduboy.justPressed(A_BUTTON)) {
    gen_mymap();
    sound.tone(300, 30);
  }

  // Process player movement + interaction
  // movement();

  // Draw the correct background for the area.
  raycast.render.drawRaycastBackground(&arduboy, bg_full);
  raycast.runIteration(&arduboy);

  FX::display(false);
}
