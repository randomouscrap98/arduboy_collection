#pragma once

constexpr uint8_t BAYERGRADIENTS = 16;

// Bayer gradients, not including the 0 fill (useless?).
// Takes up 64 precious bytes of RAM
constexpr uint8_t b_shading[] PROGMEM = {
    0xFF, 0xFF, 0xFF, 0xFF, // Beyer 16
    0xEE, 0xFF, 0xFF, 0xFF, // 0
    0xEE, 0xFF, 0xBB, 0xFF,
    0xEE, 0xFF, 0xAA, 0xFF, // 2 
    0xAA, 0xFF, 0xAA, 0xFF, 
    0xAA, 0xDD, 0xAA, 0xFF, // 4
    0xAA, 0xDD, 0xAA, 0x77,
    0xAA, 0xDD, 0xAA, 0x55, // 6
    0xAA, 0x55, 0xAA, 0x55,
    0xAA, 0x44, 0xAA, 0x55, // 8
    0xAA, 0x44, 0xAA, 0x11, 
    0xAA, 0x44, 0xAA, 0x00, // 10
    0xAA, 0x00, 0xAA, 0x00, 
    0x44, 0x00, 0xAA, 0x00, // 12
    0x44, 0x00, 0x22, 0x00,
    0x44, 0x00, 0x00, 0x00, // 14
};
