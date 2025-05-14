#include <Arduboy2.h>
#include <ArduboyTones.h>
#include <Tinyfont.h>

#include "Arduboy2Core.h"
#include "map.hpp"

Arduboy2 arduboy;
ArduboyTones sound(arduboy.audio.enabled);
RoomConfig config;
Tinyfont tinyfont =
    Tinyfont(arduboy.sBuffer, Arduboy2::width(), Arduboy2::height());

const uint8_t FRAMERATE = 30;
const uint8_t MMWIDTH = 16;
const uint8_t MMHEIGHT = 16;
const uint8_t TWIDTH = HEIGHT / MMHEIGHT;

uint8_t mymap[MMWIDTH * MMHEIGHT];

void draw_tile(uint8_t tile, uint8_t x, uint8_t y) {
  switch (tile) {
  case 1:
    arduboy.drawRect(x, y, TWIDTH, TWIDTH, WHITE);
    break;
  }
}

void draw_mymap() {
  for (int y = 0; y < MMHEIGHT; y++) {
    for (int x = 0; x < MMWIDTH; x++) {
      draw_tile(mymap[x + y * MMWIDTH], x * TWIDTH, y * TWIDTH);
    }
  }
}

void gen_mymap() {
  uint8_t a, b;
  int8_t c, d;
  genRoomsType(&config, mymap, MMWIDTH, MMHEIGHT, &a, &b, &c, &d);
}

constexpr uint8_t MENUMAX = 6;
// clang-format off
constexpr char MENULETTER[][MENUMAX] = {
  "MINWD",
  "DRBUF",
  "MXWRT",
  "9POOL",
  "CBCPL",
  "CBMXL",
  "BMPPL",
 //  "M",
 //  "D",
 //  "M",
 //  "9",
 //  "C",
 //  "C",
 //  "B",
};
// clang-format on
constexpr uint8_t MENUITEMS = sizeof(MENULETTER) / MENUMAX;

uint8_t menu_pos = 0;
void do_menu() {
  uint8_t *mval;
  char line[MENUMAX + 10];
  for (int i = 0; i < MENUITEMS; i++) {
    tinyfont.setCursor(64, i * 5);
    tinyfont.print(i == menu_pos ? '>' : ' ');
    tinyfont.print(MENULETTER[i]);
    tinyfont.print(' ');
    uint8_t *val;
    switch (i) {
    case 0:
      val = &config.minwidth;
      break;
    case 1:
      val = &config.doorbuffer;
      break;
    case 2:
      val = &config.maxwallretries;
      break;
    case 3:
      val = &config.ninepool;
      break;
    case 4:
      val = &config.cubiclepool;
      break;
    case 5:
      val = &config.cubiclemaxlength;
      break;
    case 6:
      val = &config.bumppool;
      break;
    }
    tinyfont.print(*val);
    if (i == menu_pos) {
      mval = val;
    }
  }
  int8_t dir = 0;
  if (arduboy.justPressed(DOWN_BUTTON))
    dir = 1;
  if (arduboy.justPressed(UP_BUTTON))
    dir = -1;
  menu_pos = (menu_pos + dir + MENUITEMS) % MENUITEMS;
  if (arduboy.justPressed(RIGHT_BUTTON))
    (*mval) += 1;
  if (arduboy.justPressed(LEFT_BUTTON))
    (*mval) -= 1;
  if (arduboy.justPressed(A_BUTTON)) {
    gen_mymap();
    sound.tone(300, 30);
  }
}

void setup() {
  arduboy.boot();
  arduboy.flashlight();
  arduboy.initRandomSeed();
  arduboy.setFrameRate(FRAMERATE);
  gen_mymap();
}

void loop() {
  if (!(arduboy.nextFrame())) {
    return;
  }
  arduboy.pollButtons();
  arduboy.clear();
  do_menu();
  draw_mymap();
  arduboy.display();
}
