#include "graphics.hpp"
#include "fxdata/fxdata.h"
#include "game.hpp"
#include "mymath.hpp"

// #define PRINTSTAMHEALTH
// #define PRINTSEED
// #define PRINTDIRXY

constexpr uint16_t TINYDIGITS[] PROGMEM = {
    0xF9F, 0xAF8, 0xDDA, 0x9DF, 0x74F, 0xBDD, 0xFDC, 0x11F, 0xFDF, 0x75F,
};

// Print tiny digits at given location
void print_tinydigit(uint8_t *buffer, uint8_t v, uint8_t x, uint8_t y) {
  for (uint8_t i = 0; i < 3; i++) {
    uint8_t dig = ((pgm_read_word(TINYDIGITS + v)) >> ((2 - i) * 4)) & 0xF;
    buffer[x + i + (y >> 3) * WIDTH] |= (dig << (y & 7));
  }
}

// Print a number to the given amount of digits
void print_tinynumber(uint8_t *buffer, uint16_t v, uint8_t w, uint8_t x,
                      uint8_t y) {
  for (uint8_t i = 0; i < w; i++) {
    print_tinydigit(buffer, v % 10, x + 4 * (w - i - 1), y);
    v /= 10;
  }
}

void faze_screen(uint8_t *buffer) {
  for (uint16_t i = 0; i < 1024; i++) {
    buffer[i] <<= prng() & 3;
  }
}

void clear_full_rect(uint8_t *buffer, MRect r) {
  r.y >>= 3;
  r.h >>= 3;
  for (uint8_t y = 0; y < r.h; y++) {
    for (uint8_t x = 0; x < r.w; x++) {
      buffer[x + r.x + (y + r.y) * WIDTH] = 0;
    }
  }
}

// ASSUMING THE TEXT AREA DECORATION IS KNOWN, this will very VERY quickly clear
// JUST the text area fully. It's very fast and very little code...
void prep_textarea() {
  memset(arduboy.sBuffer + (WIDTH * ((HEIGHT >> 3) - 1)), 1, WIDTH);
  tinyfont.setCursor(BMESSAGEX, BMESSAGEY);
}

uint8_t print_item_name(uint8_t item) {
  char name[16]; // Careful not to go over this!
  uint16_t offset = 0;
  FX::readDataObject<uint16_t>(itemnameoffsets + sizeof(uint16_t) * item,
                               offset);
  FX::readDataBytes(itemnames + offset, (uint8_t *)name, 16);
  tinyfont.print(name);
  return strlen(name);
}

void print_item_info(InventorySlot item) {
  if (item.count) {
    uint8_t len = print_item_name(item.item);
    if (item.count > 1) {
      tinyfont.setCursor(BMESSAGEX + (len + 1) * 5, BMESSAGEY);
      tinyfont.print(item.count);
    }
  }
}

// very optimized render image at specific address byte chunks only
void render_fximage(uint24_t addr, uint8_t *at, uint8_t width, uint8_t height) {
  height = height >> 3; // it's actually per byte ofc
  for (uint8_t i = 0; i < height; i++) {
    FX::readDataBytes(addr + 4 + i * width, at + i * WIDTH, width);
  }
}

// The fuzzy effect for specifically the title image
uint16_t fuzzy_in(uint24_t img) {
  uint16_t pending = 0;
  uint8_t b;
  for (uint16_t i = 0; i < 1024; i++) {
    FX::readDataBytes(img + 4 + i, &b, 1);
    if (arduboy.sBuffer[i] == b)
      continue;
    arduboy.sBuffer[i] = b << (prng() & 7);
    pending++;
  }
  return pending;
}

void display_menu(uint8_t mpos) {
  render_fximage(titleimg, arduboy.sBuffer, WIDTH, HEIGHT);
  tinyfont.setCursor(12, 12);
  tinyfont.print(F("NEW GAME"));
  tinyfont.setCursor(12, 18);
  tinyfont.print(arduboy.audio.enabled() ? F("SOUND:ON") : F("SOUND:OFF"));
  tinyfont.setCursor(12, 24);
  tinyfont.print(F("ABOUT"));
  tinyfont.setCursor(12, 30);
  tinyfont.print(F("QUIT"));
  tinyfont.setCursor(7, 12 + 6 * mpos);
  tinyfont.print(F("#"));
}

static void print_stat(uint8_t pos, const __FlashStringHelper *name,
                       uint16_t val) {
  uint8_t xs = 8 + (pos & 1) * 57;
  uint8_t ys = 8 + (pos >> 1) * 6;
  tinyfont.setCursor(xs, ys);
  tinyfont.print(name);
  tinyfont.setCursor(xs + 30, ys);
  tinyfont.print(val);
}

void display_about(SaveGame *sg) {
  render_fximage(blankimg, arduboy.sBuffer, WIDTH, HEIGHT);
  tinyfont.setCursor(7, 53);
  tinyfont.print(F("v107") );
  tinyfont.setCursor(47, 53);
  tinyfont.print(F("haloopdy - 2025"));
  print_stat(0, F("RUNS"), sg->total_runs);
  print_stat(1, F("WINS"), sg->total_wins);
  print_stat(2, F("ROOMS"), sg->total_rooms);
  print_stat(3, F("ENMY"), sg->total_kills);
  print_stat(4, F("ITEMS"), sg->total_items);
  print_stat(5, F("USED"), sg->total_used);
  const __FlashStringHelper *rgns[] = {
      F("RGN0"),
      F("RGN1"),
      F("RGN2"),
      F("RGN3"),
  };
  for (uint8_t i = 0; i < 4; i++) {
    print_stat(6 + i, rgns[i], sg->completed_region[i]);
  }
  print_stat(10, F("MTIME"), sg->total_seconds / 60);
  print_stat(11, F("SEED"), sg->player_seed);
}

void draw_full_map(GameState *gs, uint8_t xs, uint8_t ys) {
  arduboy.fillRect(xs, ys, gs->map.width, gs->map.height, BLACK);
  for (int y = 0; y < gs->map.height; y++) {
    for (int x = 0; x < gs->map.width; x++) {
      if (MAPT(gs->map, x, gs->map.height - 1 - y)) {
        arduboy.drawPixel(xs + x, ys + y, WHITE);
      }
    }
  }
}

void draw_map_near(GameState *gs, uint8_t xs, uint8_t ys, uint8_t range) {
  for (int8_t y = -range; y <= range; y++) {
    for (int8_t x = -range; x <= range; x++) {
      uint8_t px = x + gs->player.posX;
      uint8_t py = y + gs->player.posY;
      if (px >= gs->map.width || py >= gs->map.height)
        continue;
      uint8_t mt = MAPT(gs->map, px, py);
      arduboy.drawPixel(xs + px, ys + gs->map.height - 1 - py,
                        mt ? WHITE : BLACK);
    }
  }
}

void draw_map_player(GameState *gs, uint8_t xs, uint8_t ys, uint8_t col) {
  arduboy.drawPixel(xs + gs->player.posX,
                    ys + gs->map.height - 1 - gs->player.posY, col);
}

// Visual only?
constexpr uint8_t ITEMMENUEDGE = 97;
constexpr uint8_t ITEMMENUTOP = 26;
constexpr uint8_t ITEMMENUHEIGHT = 30;
constexpr uint8_t ITEMCURSORSTARTX = 98;
constexpr uint8_t ITEMCURSORSTARTY = 27;
constexpr uint8_t ITEMCURSORMOVEX = 9;
constexpr uint8_t ITEMCURSORMOVEY = 9;
constexpr uint8_t ITEMSTARTX = 99;
constexpr uint8_t ITEMSTARTY = 28;
constexpr uint8_t ITEMSCROLLBARHEIGHT = 8;
constexpr uint8_t ITEMSCROLLTOP = ITEMMENUTOP + 1;
constexpr uint8_t ITEMSCROLLHEIGHT = ITEMMENUHEIGHT - 2;

// Clear ONLY the items menu quickly
void static clear_items_menu() {
  arduboy.fillRect(ITEMMENUEDGE, ITEMMENUTOP, WIDTH - ITEMMENUEDGE,
                   ITEMMENUHEIGHT, BLACK);
}

// Fully redraw the items menu, including cursor at given position. If cursor
// position is off screen, will not draw cursor
void draw_items_menu_w_cursor(GameState *gs, uint8_t vpos, bool selected) {
  clear_items_menu();
  if (vpos < ITEMSPAGE) {
    FX::drawBitmap(ITEMCURSORSTARTX + ITEMCURSORMOVEX * (vpos % ITEMSACROSS),
                   ITEMCURSORSTARTY + ITEMCURSORMOVEY * (vpos / ITEMSDOWN),
                   selected ? cursorselectimg : cursorimg, 0, dbmOverwrite);
  }
  for (uint8_t i = 0; i < ITEMSPAGE; i++) {
    if (gs_has_item(gs, gs->item_top + i)) {
      uint8_t item = gs->inventory[gs->item_top + i].item;
      FX::drawBitmap(ITEMSTARTX + ITEMCURSORMOVEX * (i % ITEMSACROSS),
                     ITEMSTARTY + ITEMCURSORMOVEY * (i / ITEMSDOWN), itemsheet,
                     item, dbmOverwrite);
    }
  }
  // Don't forget to draw the scrollbar
  uint8_t range = gs->max_items - ITEMSPAGE;
  if (range != 0) {
    float fscrollpos = (float)gs->item_top / range;
    arduboy.fillRect(WIDTH - 1,
                     ITEMSCROLLTOP +
                         (ITEMSCROLLHEIGHT - ITEMSCROLLBARHEIGHT) * fscrollpos,
                     1, ITEMSCROLLBARHEIGHT);
  }
}

void draw_items_menu(GameState *gs) {
  draw_items_menu_w_cursor(gs, 255, false);
}

// void draw_items_menu_w_cursor(GameState *gs, uint8_t vpos, bool selected) {
//   clear_items_menu();
//   // Must draw cursor first, then items. Ah well, could fix it maybe
//   //((arduboy.buttonsState() & ITEMS_SWAPBTN) == ITEMS_SWAPBTN)
//   //((arduboy.buttonsState() & ITEMS_SWAPBTN) == ITEMS_SWAPBTN)
//   draw_items_menu(gs);
// }

// Draw a standard 2x16 bar at given x, y chosen by default
void draw_std_bar(uint8_t x, uint8_t filled, uint8_t max) {
  uint8_t fh = round(BARHEIGHT * (float)filled / (float)max);
  arduboy.fillRect(x, BARTOP, 2, BARHEIGHT, BLACK);
  arduboy.fillRect(x, BARTOP + BARHEIGHT - fh, 2, fh, WHITE);
}

void draw_runtime_data(GameState *gs) {
  draw_std_bar(HEALTHBARX, gs->health, BASESTAMHEALTH);
  draw_std_bar(STAMINABARX, gs->stamina, BASESTAMHEALTH);
  draw_map_near(gs, MAPX, MAPY, MAPRANGE);
  draw_map_player(gs, MAPX, MAPY,
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
