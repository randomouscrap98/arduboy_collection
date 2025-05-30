#pragma once

// JUST TO SHUTUP CLANGD
#ifdef __clang__
#define __uint24 uint32_t
#endif

// #include "mymath.hpp"
#include "game.hpp"
#include <Arduboy2.h>
#include <ArduboyFX.h>
#include <Tinyfont.h>

extern Arduboy2Base arduboy;
extern Tinyfont tinyfont;

constexpr uint8_t BOTTOMSIZE = 8;
constexpr uint8_t SIDESIZE = 32;
constexpr uint8_t BMESSAGEX = 1;
constexpr uint8_t BMESSAGEY = HEIGHT - BOTTOMSIZE + 2;

constexpr uint8_t BARWIDTH = 2;
constexpr uint8_t BARHEIGHT = 16;
constexpr uint8_t BARTOP = 2;

void print_tinydigit(uint8_t *buffer, uint8_t v, uint8_t x, uint8_t y);
void print_tinynumber(uint8_t *buffer, uint16_t v, uint8_t w, uint8_t x,
                      uint8_t y);

void faze_screen(uint8_t *buffer);
uint16_t fuzzy_in(uint24_t img);

void prep_textarea();
uint8_t print_item_name(uint8_t item);
void print_item_info(InventorySlot item);
// void print_item_info(uint8_t item, uint8_t count);
void render_fximage(uint24_t addr, uint8_t *at, uint8_t width, uint8_t height);

void display_menu(uint8_t mpos);
void display_about(SaveGame *sg);

void draw_full_map(GameState *gs, uint8_t xs, uint8_t ys);
void draw_map_near(GameState *gs, uint8_t xs, uint8_t ys, uint8_t range);
void draw_map_player(GameState *gs, uint8_t xs, uint8_t ys, uint8_t col);

void clear_items_menu();
void draw_items_menu(GameState *gs);
void draw_items_menu_w_cursor(GameState *gs, uint8_t vpos, bool selected);
void draw_std_bar(uint8_t x, uint8_t filled, uint8_t max);
