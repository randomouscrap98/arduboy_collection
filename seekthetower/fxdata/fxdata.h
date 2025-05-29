#pragma once

#include <Arduboy2.h>
using uint24_t = __uint24;

// Generated with ardugotools on 2025-05-28T23:24:00-04:00

// Image info for "tilesheet"
// Raycast frame bytes: 172, mipmap widths: 32,16,8,4
constexpr uint24_t tilesheet       = 0x000000;
constexpr uint8_t  tilesheetFrames = 256;
// Image info for "spritesheet"
// Raycast frame bytes: 172, mipmap widths: 32,16,8,4
constexpr uint24_t spritesheet       = 0x00AC00;
constexpr uint24_t spritesheetMask   = 0x015800;
constexpr uint8_t  spritesheetFrames = 256;
// Image info for "menu"
constexpr uint24_t menu       = 0x020400;
constexpr uint8_t  menuFrames = 1;
constexpr uint16_t menuWidth  = 128;
constexpr uint16_t menuHeight = 64;
// Image info for "bg"
constexpr uint24_t bg       = 0x020804;
constexpr uint8_t  bgFrames = 1;
constexpr uint16_t bgWidth  = 96;
constexpr uint16_t bgHeight = 56;
// Image info for "titleimg"
constexpr uint24_t titleimg       = 0x020AA8;
constexpr uint8_t  titleimgFrames = 1;
constexpr uint16_t titleimgWidth  = 128;
constexpr uint16_t titleimgHeight = 64;
// Image info for "blankimg"
constexpr uint24_t blankimg       = 0x020EAC;
constexpr uint8_t  blankimgFrames = 1;
constexpr uint16_t blankimgWidth  = 128;
constexpr uint16_t blankimgHeight = 64;
// Image info for "cursorimg"
constexpr uint24_t cursorimg       = 0x0212B0;
constexpr uint8_t  cursorimgFrames = 1;
constexpr uint16_t cursorimgWidth  = 10;
constexpr uint16_t cursorimgHeight = 10;
// Image info for "cursorselectimg"
constexpr uint24_t cursorselectimg       = 0x0212C8;
constexpr uint8_t  cursorselectimgFrames = 1;
constexpr uint16_t cursorselectimgWidth  = 10;
constexpr uint16_t cursorselectimgHeight = 10;
// Image info for "itemsheet"
constexpr uint24_t itemsheet       = 0x0212E0;
constexpr uint8_t  itemsheetFrames = 256;
constexpr uint16_t itemsheetWidth  = 8;
constexpr uint16_t itemsheetHeight = 8;

// FX addresses (only really used for initialization)
constexpr uint16_t FX_DATA_PAGE = 0xFDD5;
constexpr uint24_t FX_DATA_BYTES = 137956;

constexpr uint16_t FX_SAVE_PAGE = 0xFFF0;
constexpr uint24_t FX_SAVE_BYTES = 0;

// Helper macro to initialize fx, call in setup()
#define FX_INIT() FX::begin(FX_DATA_PAGE, FX_SAVE_PAGE)
