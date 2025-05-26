#pragma once

using uint24_t = __uint24;

// Generated with ardugotools on 2025-05-26T17:43:47-04:00

// Image info for "tilesheet"
// Raycast frame bytes: 172, mipmap widths: 32,16,8,4
constexpr uint24_t tilesheet       = 0x000000;
constexpr uint8_t  tilesheetFrames = 256;
// Image info for "menu"
constexpr uint24_t menu       = 0x00AC00;
constexpr uint8_t  menuFrames = 1;
constexpr uint16_t menuWidth  = 128;
constexpr uint16_t menuHeight = 64;
// Image info for "bg"
constexpr uint24_t bg       = 0x00B004;
constexpr uint8_t  bgFrames = 1;
constexpr uint16_t bgWidth  = 96;
constexpr uint16_t bgHeight = 56;
// Image info for "titleimg"
constexpr uint24_t titleimg       = 0x00B2A8;
constexpr uint8_t  titleimgFrames = 1;
constexpr uint16_t titleimgWidth  = 128;
constexpr uint16_t titleimgHeight = 64;
// Image info for "blankimg"
constexpr uint24_t blankimg       = 0x00B6AC;
constexpr uint8_t  blankimgFrames = 1;
constexpr uint16_t blankimgWidth  = 128;
constexpr uint16_t blankimgHeight = 64;
// Image info for "cursor"
constexpr uint24_t cursor       = 0x00BAB0;
constexpr uint8_t  cursorFrames = 1;
constexpr uint16_t cursorWidth  = 10;
constexpr uint16_t cursorHeight = 10;
// Image info for "items"
constexpr uint24_t items       = 0x00BAC8;
constexpr uint8_t  itemsFrames = 256;
constexpr uint16_t itemsWidth  = 10;
constexpr uint16_t itemsHeight = 10;

// FX addresses (only really used for initialization)
constexpr uint16_t FX_DATA_PAGE = 0xFF21;
constexpr uint24_t FX_DATA_BYTES = 52940;

constexpr uint16_t FX_SAVE_PAGE = 0xFFF0;
constexpr uint24_t FX_SAVE_BYTES = 0;

// Helper macro to initialize fx, call in setup()
#define FX_INIT() FX::begin(FX_DATA_PAGE, FX_DATA_SAVE)
