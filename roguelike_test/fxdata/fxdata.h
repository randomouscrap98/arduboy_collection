#pragma once

using uint24_t = __uint24;

// Generated with ardugotools on 2025-05-19T01:10:20-04:00

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

// FX addresses (only really used for initialization)
constexpr uint16_t FX_DATA_PAGE = 0xFF4D;
constexpr uint24_t FX_DATA_BYTES = 45736;

// Helper macro to initialize fx, call in setup()
#define FX_INIT() FX::begin(FX_DATA_PAGE)
