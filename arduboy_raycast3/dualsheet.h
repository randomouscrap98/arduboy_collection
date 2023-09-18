#pragma once

constexpr uint8_t dualsheetWidth = 16;
constexpr uint8_t dualsheetHeight = 16;

#include "rcmap.h"

//All tiles should be 8 bytes I think
constexpr uint8_t TILEEMPTY = 0;
constexpr uint8_t TILEWALL = 3;
constexpr uint8_t TILEEXIT = 50;
constexpr uint8_t TILEDOOR = 49;

constexpr uint8_t SPRITEMONSTER = 44;
constexpr uint8_t SPRITELEVER = 45;
constexpr uint8_t SPRITEBARREL = 60;
constexpr uint8_t SPRITECHEST = 61;

constexpr uint8_t TILESIZE = 16;

inline bool isCellSolid(RcMap * map, uint8_t x, uint8_t y)
{
    uint8_t cell = getMapCell(map, x, y);
    return cell != 0 && (cell < 6 || cell > 48);
}


constexpr uint8_t dualsheet[] PROGMEM
{
  //Frame 0
  0x01, 0xFE, 0xFA, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFA, 0xFE, 0x01, 
  0x80, 0x7F, 0x5F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x5F, 0x7F, 0x80, 

  //Frame 1
  0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 
  0x0E, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xE0, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 0xEE, 

  //Frame 2
  0x01, 0xFE, 0xAA, 0x56, 0xAA, 0x56, 0xAA, 0x56, 0xAA, 0x56, 0xAA, 0x56, 0xAA, 0x56, 0x02, 0x01, 
  0x80, 0x7F, 0x2A, 0x15, 0x2A, 0x15, 0x2A, 0x15, 0x2A, 0x15, 0x2A, 0x15, 0x2A, 0x15, 0x00, 0x80, 

  //Frame 3
  0x06, 0xEA, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 0x60, 0xAE, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 
  0x06, 0xEA, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 0x60, 0xAE, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 

  //Frame 4
  0x01, 0xFE, 0x02, 0x0A, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x0A, 0x02, 0x00, 0x01, 
  0x80, 0x3F, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x80, 

  //Frame 5
  0xE3, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 
  0xE3, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 0x3E, 0x22, 0x22, 0x22, 0x22, 0x22, 0x22, 

  //Frame 6
  0xFF, 0xFF, 0xFF, 0xEF, 0x9F, 0xFF, 0xFF, 0xFF, 0xF3, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0xFF, 
  0xFF, 0xFF, 0xEF, 0xDF, 0xFF, 0xFF, 0xF9, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xCE, 0xFF, 0xFF, 

  //Frame 7
  0xFF, 0x3F, 0xEF, 0xD3, 0xB5, 0xD7, 0xFA, 0xDE, 0xE7, 0x8E, 0xC2, 0x01, 0x83, 0x97, 0x5F, 0xBF, 
  0xFC, 0xFB, 0xF5, 0xEE, 0x77, 0x34, 0x03, 0x03, 0x00, 0x04, 0x20, 0x60, 0xE8, 0xF6, 0xF0, 0xF8, 

  //Frame 8
  0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x7F, 0x7F, 0x1F, 0x5F, 0x8F, 0x4F, 0xA7, 0x57, 0xA7, 0x53, 
  0xFF, 0xFF, 0x03, 0x14, 0xAA, 0x55, 0xAA, 0x5D, 0xAA, 0x5D, 0xAA, 0x55, 0xAB, 0x55, 0xBB, 0x55, 

  //Frame 9
  0xAB, 0x53, 0xA3, 0x57, 0xA7, 0x47, 0xAF, 0x4F, 0xA7, 0x57, 0xAB, 0x53, 0xAB, 0xD3, 0xAB, 0x53, 
  0xBA, 0x55, 0xAA, 0x55, 0xAA, 0x75, 0xAA, 0x77, 0xAA, 0x75, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 

  //Frame 10
  0xA7, 0x57, 0xA3, 0x53, 0xAB, 0xD3, 0xAB, 0xD3, 0xA3, 0x4F, 0x8F, 0x5F, 0x9F, 0x3F, 0x7F, 0xFF, 
  0xAA, 0x55, 0xAE, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x77, 0xEA, 0x57, 0xAA, 0x05, 0xF0, 0xFF, 

  //Frame 11
  0xFE, 0xEE, 0xE6, 0x0E, 0xEA, 0x60, 0xAE, 0x0A, 0xE6, 0x22, 0xA0, 0x06, 0x22, 0x00, 0x22, 0x00, 
  0xFE, 0xEE, 0xE6, 0x0E, 0xEA, 0x60, 0xAE, 0x0A, 0xE6, 0x22, 0xA0, 0x06, 0x22, 0x00, 0x22, 0x00, 

  //Frame 12
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFE, 0xEE, 0xE0, 0x0F, 0xEF, 0x60, 0xAE, 0x0A, 0xE6, 0x22, 0xA0, 0x0F, 0x2F, 0x00, 0x22, 0x00, 

  //Frame 13
  0xFF, 0xFF, 0xDF, 0xCF, 0xC7, 0xE3, 0xE3, 0xE1, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFE, 0xEE, 0xE0, 0x0F, 0xEF, 0x60, 0xAE, 0x0A, 0xE6, 0x22, 0xA0, 0x0F, 0x2F, 0x00, 0x22, 0x00, 

  //Frame 14
  0xFF, 0x7F, 0xBF, 0xBF, 0x83, 0xBB, 0xA8, 0xBA, 0xAA, 0xA8, 0xAB, 0x3B, 0xAB, 0xBB, 0xD7, 0xEF, 
  0xFE, 0xFD, 0xFB, 0xDA, 0x3B, 0xFA, 0x03, 0xF2, 0x52, 0x02, 0xFB, 0x78, 0xBF, 0xFF, 0xFF, 0xFF, 

  //Frame 15
  0xF7, 0xF3, 0x39, 0x19, 0x89, 0xC1, 0xC1, 0xE3, 0x03, 0xA1, 0x01, 0xC0, 0xC8, 0x9C, 0x3C, 0xFD, 
  0xFF, 0xFF, 0xFE, 0xFF, 0x7F, 0x3F, 0x1F, 0x27, 0x50, 0x2A, 0x40, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Frame 16
  0xFF, 0xFF, 0xF8, 0x31, 0x82, 0x55, 0xAA, 0x55, 0xAA, 0x5D, 0xAA, 0xDD, 0xAA, 0xD5, 0xAA, 0x55, 
  0xFF, 0xFF, 0x80, 0x54, 0xAA, 0x57, 0xAA, 0x57, 0xAA, 0x5D, 0xAA, 0x5D, 0xAA, 0x5D, 0xAA, 0xD5, 

  //Frame 17
  0xAA, 0x55, 0xAE, 0x55, 0xAA, 0x55, 0xEA, 0x55, 0xEA, 0x55, 0xEA, 0x7D, 0xAA, 0x5D, 0xAA, 0x55, 
  0xAA, 0xD7, 0xAA, 0x57, 0xBA, 0x55, 0xBA, 0x55, 0xAE, 0x55, 0xAE, 0x55, 0xEE, 0x55, 0xEA, 0x55, 

  //Frame 18
  0xAB, 0x55, 0xEB, 0x55, 0xEA, 0x55, 0xAE, 0x55, 0xAA, 0x55, 0x0A, 0xC5, 0xE0, 0xFC, 0xFF, 0xFF, 
  0xAA, 0x57, 0xAA, 0x77, 0xAA, 0x75, 0xAA, 0x55, 0xEA, 0x55, 0xE8, 0x51, 0x87, 0x1F, 0xFF, 0xFF, 

  //Frame 19
  0x03, 0xEF, 0xEF, 0xEF, 0x03, 0xEF, 0xEF, 0xEF, 0x03, 0xEF, 0xEF, 0xEF, 0x03, 0xEF, 0xEF, 0xEF, 
  0x00, 0xBF, 0xBF, 0xBF, 0x00, 0xBF, 0xBF, 0xBF, 0x00, 0xBF, 0xBF, 0xBF, 0x00, 0xBF, 0xBF, 0xBF, 

  //Frame 20
  0x03, 0xEF, 0xF7, 0xF7, 0x01, 0xFB, 0x7D, 0x00, 0x7F, 0x00, 0x7D, 0xFB, 0x01, 0xF7, 0xF7, 0xEF, 
  0x00, 0xBF, 0xBF, 0xBF, 0x00, 0xBF, 0xB8, 0x00, 0xF8, 0x00, 0xB8, 0xBF, 0x00, 0xBF, 0xBF, 0xBF, 

  //Frame 21
  0xFF, 0xFF, 0x9F, 0xBB, 0x77, 0xF1, 0x83, 0x0D, 0x3F, 0x1C, 0xC3, 0xFD, 0xFF, 0x67, 0x1F, 0xEF, 
  0xFB, 0xFA, 0xFD, 0xFD, 0xFE, 0x7C, 0x39, 0x00, 0x00, 0x00, 0x7D, 0xFD, 0xFA, 0xFB, 0xFD, 0xFF, 

  //Frame 22
  0xFF, 0xFF, 0xFF, 0x1F, 0xE7, 0x1B, 0xAB, 0x85, 0xED, 0x85, 0xAB, 0x1B, 0xE7, 0x1F, 0xFF, 0xFF, 
  0xFF, 0xBF, 0x7F, 0x00, 0xFF, 0x54, 0x2A, 0x40, 0xAF, 0x40, 0x2A, 0x14, 0xFF, 0x00, 0x3F, 0xDF, 

  //Frame 23
  0xFF, 0xFF, 0xFF, 0x8F, 0xAF, 0xAF, 0x21, 0xFD, 0x21, 0xAF, 0xAF, 0x8F, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xBF, 0x7F, 0xFF, 0x07, 0xFB, 0xAB, 0x58, 0xAF, 0x58, 0xAB, 0x5B, 0x07, 0xFF, 0x3F, 0xDF, 0xFF, 

  //Frame 24
  0xFF, 0xFF, 0x1F, 0x40, 0xAA, 0x55, 0xAE, 0x55, 0xAE, 0x55, 0xEA, 0x55, 0xEA, 0x55, 0xAA, 0x57, 
  0xFF, 0xFF, 0xFC, 0xF9, 0xFA, 0xE1, 0xCB, 0xD5, 0xCB, 0x95, 0xAA, 0x95, 0xAA, 0x95, 0x8A, 0xD5, 

  //Frame 25
  0xAA, 0x57, 0xBA, 0x55, 0xBA, 0x55, 0xAA, 0xD5, 0xAA, 0xDD, 0xAA, 0x5D, 0xAA, 0x55, 0xAA, 0x55, 
  0xCA, 0xE5, 0xF2, 0xF1, 0xFA, 0xF9, 0xF2, 0xF5, 0xEA, 0xE5, 0xEA, 0xE5, 0xEA, 0xC7, 0xCA, 0xC5, 

  //Frame 26
  0xBA, 0x55, 0xBA, 0x55, 0xAA, 0x55, 0xAE, 0x55, 0xAE, 0x55, 0x0A, 0xE5, 0xF2, 0xF8, 0xFF, 0xFF, 
  0xEA, 0xE5, 0xF2, 0xF5, 0xF2, 0xF5, 0xF0, 0xFD, 0xFC, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Frame 27
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

  //Frame 28
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xBF, 0xAF, 0x0B, 0x01, 0x17, 0x37, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xD7, 0x91, 0xD2, 0xC2, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x82, 0x93, 0xD7, 0xFF, 

  //Frame 29
  0xC0, 0xAF, 0x32, 0x75, 0xAA, 0x80, 0xDA, 0xDA, 0x00, 0x3E, 0x95, 0x49, 0x60, 0x5E, 0x15, 0x0A, 
  0x7F, 0xAA, 0xD5, 0xC0, 0xBE, 0x7D, 0xA9, 0x55, 0x28, 0x80, 0xDF, 0xBF, 0x55, 0xAA, 0x55, 0x80, 

  //Frame 30
  0xC0, 0xAF, 0x32, 0x75, 0xAA, 0x80, 0xDA, 0xDA, 0x80, 0xBE, 0x95, 0xC9, 0xE0, 0xFE, 0xFE, 0xFF, 
  0x7F, 0xAA, 0xD5, 0xC0, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Frame 31
  0xFC, 0xFB, 0xF2, 0xF5, 0xEA, 0xF4, 0xFA, 0xF0, 0x80, 0x3E, 0x95, 0x49, 0x60, 0x5E, 0x15, 0x0A, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xE0, 0xDF, 0xBF, 0x95, 0xAA, 0x55, 0x80, 

  //Frame 32
  0xAA, 0x55, 0xAA, 0x75, 0xAE, 0x75, 0xAE, 0x55, 0xAE, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 
  0xAA, 0x55, 0xAA, 0x75, 0xAA, 0x75, 0xAA, 0x55, 0xAB, 0x55, 0xAB, 0x55, 0x0B, 0x75, 0xE2, 0xC5, 

  //Frame 33
  0xEA, 0x55, 0xEE, 0x55, 0xAE, 0x5D, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x5D, 0xAA, 0x55, 
  0xCA, 0xD5, 0x2A, 0x55, 0xAA, 0x55, 0xAB, 0x75, 0xAB, 0x75, 0xAB, 0x55, 0xAA, 0x55, 0xAA, 0x55, 

  //Frame 34
  0x00, 0xE6, 0xF2, 0xF8, 0xFC, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFC, 0xF8, 0xF2, 0xE6, 0x00, 
  0x06, 0xEA, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 0x60, 0xAE, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 

  //Frame 35
  0xD0, 0x76, 0x86, 0x76, 0xD0, 0x76, 0x86, 0x76, 0xD0, 0x76, 0x86, 0x76, 0xD0, 0x76, 0x86, 0x76, 
  0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 

  //Frame 36
  0xFF, 0xBF, 0xFF, 0xDB, 0xAF, 0x8F, 0x9F, 0xFF, 0x7F, 0x7E, 0xC7, 0xDB, 0xCB, 0xC7, 0xFF, 0xFF, 
  0xFF, 0xE7, 0xDB, 0xCB, 0xC7, 0xEF, 0xF9, 0xF6, 0xF5, 0xB2, 0xF0, 0xF1, 0xFF, 0xFF, 0xFF, 0x7F, 

  //Frame 37
  0xFF, 0xFF, 0xFF, 0xFF, 0x3F, 0xCF, 0x5F, 0x3F, 0xC7, 0xF3, 0x61, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xEF, 0xD1, 0xCC, 0xD7, 0xAA, 0xB1, 0xBE, 0x97, 0xAA, 0x94, 0x80, 0xC1, 0xC7, 0xFF, 0xFF, 

  //Frame 38
  0xC0, 0xAE, 0x32, 0x75, 0xAB, 0x83, 0xDF, 0xDF, 0x1F, 0x3F, 0xBF, 0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 
  0x7F, 0xAA, 0xD5, 0xC0, 0xBE, 0x7D, 0xA9, 0x55, 0x28, 0x80, 0xDF, 0xBF, 0x55, 0xAA, 0x55, 0x80, 

  //Frame 39
  0xFF, 0xFF, 0xFF, 0x7F, 0xBF, 0x9F, 0xDF, 0xDF, 0x1F, 0x3F, 0x9F, 0x4F, 0x63, 0x5B, 0x15, 0x0A, 
  0x7F, 0xBF, 0xDF, 0xC0, 0xBE, 0x7D, 0xA9, 0x55, 0x28, 0x80, 0xDF, 0xBF, 0x55, 0xAA, 0x55, 0x80, 

  //Frame 40
  0xAA, 0x75, 0xAA, 0x75, 0xAA, 0x55, 0xAA, 0x55, 0xAE, 0x55, 0xAA, 0xD5, 0xAA, 0xD4, 0xA9, 0x53, 
  0xAA, 0x55, 0xEA, 0x55, 0xEE, 0x55, 0xAE, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0xD7, 

  //Frame 41
  0xA3, 0x53, 0xA8, 0x55, 0xAA, 0x5D, 0xAA, 0x5D, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x75, 0xEB, 0x55, 
  0xAA, 0xD7, 0xAA, 0x57, 0xAA, 0x5D, 0xAA, 0x5D, 0xAA, 0x55, 0xAA, 0x75, 0xAA, 0x75, 0xAA, 0x55, 

  //Frame 42
  0xFF, 0xFF, 0xFF, 0xFF, 0x1F, 0xE7, 0xF9, 0x5E, 0xBE, 0x59, 0xA7, 0x1F, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xBF, 0x7F, 0xFF, 0xFF, 0x80, 0xFF, 0xFF, 0x01, 0x2A, 0xD5, 0xAA, 0x80, 0xFF, 0x3F, 0xDF, 0xFF, 

  //Frame 43
  0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 
  0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 0xD8, 0x77, 0x8D, 0x77, 

  //Frame 44
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x20, 0x40, 0x00, 0x40, 0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

  //Frame 45
  0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x1C, 0x14, 0x14, 0x10, 0x10, 0x14, 0x14, 0x14, 0x14, 0x1C, 0x00, 0x00, 0x00, 

  //Frame 46
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x1C, 0x14, 0x14, 0x14, 0x14, 0x10, 0x10, 0x14, 0x14, 0x1C, 0x00, 0x00, 0x00, 

  //Frame 47
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

  //Frame 48
  0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 
  0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 0xAA, 0x55, 

  //Frame 49
  0x06, 0xEA, 0x06, 0xFA, 0xFC, 0xFC, 0x3C, 0x1C, 0x1C, 0x3C, 0xFC, 0xFC, 0xFA, 0x02, 0x66, 0xAA, 
  0x06, 0xEA, 0x00, 0x7F, 0x7F, 0x7F, 0x73, 0x70, 0x70, 0x73, 0x7F, 0x7F, 0x7F, 0x00, 0x66, 0xAA, 

  //Frame 50
  0x06, 0xEA, 0x06, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x02, 0x66, 0xAA, 
  0x06, 0xEA, 0x00, 0x00, 0x80, 0x80, 0x80, 0xC0, 0xC0, 0xC0, 0xE0, 0xE0, 0xE0, 0x00, 0x66, 0xAA, 

  //Frame 51
  0x06, 0xEA, 0x66, 0xA0, 0x6F, 0xAF, 0x6F, 0xAF, 0x6F, 0xAF, 0x6F, 0xAF, 0x60, 0xAE, 0x66, 0xAA, 
  0x06, 0xEA, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 0x60, 0xAE, 0x66, 0xAA, 0x66, 0xAA, 0x66, 0xAA, 

  //Frame 52
  0x06, 0xEA, 0x66, 0xEA, 0x3E, 0x8E, 0x06, 0xF2, 0x02, 0xE6, 0x0E, 0x3A, 0xE6, 0xAA, 0x66, 0xAA, 
  0x06, 0xEA, 0x66, 0xDF, 0x40, 0x5F, 0x41, 0x5F, 0x41, 0x5F, 0x41, 0x40, 0x5F, 0x2A, 0x66, 0xAA, 

  //Frame 53
  0x00, 0x00, 0xFF, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xFF, 0x00, 0x00, 
  0x00, 0x00, 0xFF, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xFF, 0x00, 0x00, 

  //Frame 54
  0x3F, 0xBF, 0xBF, 0xBF, 0x0F, 0xEF, 0xEF, 0xEF, 0x03, 0xFB, 0xFB, 0xFB, 0x00, 0x7E, 0x7E, 0x7E, 
  0x00, 0x5F, 0x1F, 0x5F, 0x00, 0x57, 0x27, 0x57, 0x20, 0x55, 0x29, 0x55, 0x28, 0x55, 0x2A, 0x00, 

  //Frame 55
  0xFA, 0xF8, 0xFA, 0x00, 0xEA, 0xE4, 0xEA, 0x04, 0xAA, 0x94, 0xAA, 0x14, 0xAA, 0x54, 0xAA, 0x00, 
  0x7F, 0x7F, 0x7F, 0x00, 0x7F, 0x7F, 0x7F, 0x00, 0x7F, 0x7F, 0x7F, 0x00, 0x7E, 0x7E, 0x7E, 0x00, 

  //Frame 56
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

  //Frame 57
  0x00, 0x00, 0x00, 0x00, 0x80, 0x60, 0xB0, 0x50, 0x70, 0x70, 0xE0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x20, 0x14, 0x02, 0x09, 0x0E, 0x0F, 0x06, 0x02, 0x03, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 

  //Frame 58
  0x00, 0x00, 0x00, 0x00, 0xF0, 0xD8, 0xA8, 0x5C, 0xBC, 0x54, 0xA8, 0x50, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x01, 0x07, 0x03, 0x06, 0x01, 0x02, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 

  //Frame 59
  0x00, 0xE0, 0x28, 0x2C, 0xAE, 0xAA, 0x2A, 0xAA, 0xAA, 0x2A, 0xEA, 0x0E, 0x56, 0x0A, 0x54, 0x00, 
  0x00, 0x7F, 0x40, 0x5B, 0x4D, 0x56, 0x5B, 0x4D, 0x56, 0x40, 0x7F, 0x00, 0x15, 0x08, 0x05, 0x00, 

  //Frame 60
  0x00, 0x00, 0x00, 0xD0, 0xB4, 0xB4, 0x6E, 0x2E, 0x4E, 0x2E, 0x54, 0x84, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x07, 0x37, 0x2F, 0x4F, 0x0A, 0x55, 0x0A, 0x15, 0x02, 0x00, 0x00, 0x00, 0x00, 

  //Frame 61
  0x00, 0x70, 0x40, 0x50, 0x40, 0x50, 0x40, 0x50, 0x40, 0x50, 0x40, 0x50, 0x40, 0x40, 0x70, 0x00, 
  0x00, 0x7F, 0x01, 0x55, 0x29, 0x41, 0x2F, 0x49, 0x29, 0x4F, 0x21, 0x55, 0x29, 0x01, 0x7F, 0x00, 

  //Frame 62
  0x00, 0x7E, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x7E, 0x00, 
  0x00, 0x7F, 0x01, 0x55, 0x29, 0x41, 0x2F, 0x49, 0x29, 0x4F, 0x21, 0x55, 0x29, 0x01, 0x7F, 0x00, 

  //Frame 63
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

constexpr uint8_t dualsheet_Mask[] PROGMEM
{
  //Mask Frame 0
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 1
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 2
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 3
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 4
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 5
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 6
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 7
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 8
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 9
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 10
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 11
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 12
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 13
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 14
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 15
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 16
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 17
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 18
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 19
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 20
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 21
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 22
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 23
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 24
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 25
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 26
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 27
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 28
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 29
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 30
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 31
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 32
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 33
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 34
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 35
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 36
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 37
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 38
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 39
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 40
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 41
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 42
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 43
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 44
  0x00, 0x50, 0x30, 0xF0, 0xE4, 0xFA, 0xF0, 0xF8, 0xF8, 0xF8, 0xF0, 0xFA, 0xE4, 0xF0, 0x30, 0x50, 
  0x00, 0x3D, 0x0F, 0xC7, 0x61, 0xFD, 0x7F, 0xDF, 0x1F, 0xDF, 0x7F, 0xFD, 0x61, 0xC7, 0x0F, 0x3D, 

  //Mask Frame 45
  0x00, 0x0C, 0x1E, 0x3E, 0xFC, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x00, 0x00, 

  //Mask Frame 46
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0xFC, 0x3E, 0x1E, 0x0C, 0x00, 
  0x00, 0x00, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3E, 0x3F, 0x3F, 0x3E, 0x3E, 0x3E, 0x00, 0x00, 

  //Mask Frame 47
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0xC0, 0x78, 0x7C, 0x7C, 0x7C, 0x38, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x0C, 0x1E, 0x2F, 0x05, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 

  //Mask Frame 48
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 49
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 50
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 51
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 52
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 53
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 54
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 55
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 56
  0x00, 0x00, 0x00, 0x00, 0x38, 0x78, 0x60, 0xFE, 0xFF, 0xFE, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x20, 0xC0, 0x00, 0xFF, 0xFF, 0xFF, 0x83, 0x43, 0x01, 0x00, 0x00, 0x00, 

  //Mask Frame 57
  0x00, 0x06, 0x0C, 0x9C, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF0, 0xE0, 0xE0, 0x60, 0x60, 0x30, 
  0x30, 0x7C, 0x3E, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x1F, 0x1B, 0x10, 0x10, 0x00, 0x00, 

  //Mask Frame 58
  0x00, 0x00, 0xC0, 0xF0, 0xF8, 0xFC, 0xFC, 0xFE, 0xFE, 0xFE, 0xFC, 0xFC, 0xF8, 0xE0, 0x00, 0x00, 
  0x00, 0x00, 0x03, 0x07, 0x1F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x3F, 0x1F, 0x1F, 0x07, 0x00, 0x00, 

  //Mask Frame 59
  0xF0, 0xF8, 0xFC, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x3F, 0x1F, 0x0F, 

  //Mask Frame 60
  0x00, 0x00, 0xF0, 0xFC, 0xFE, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE, 0xFE, 0xFC, 0xF0, 0x00, 0x00, 
  0x00, 0x00, 0x0F, 0x3F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x7F, 0x3F, 0x0F, 0x00, 0x00, 

  //Mask Frame 61
  0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 0xF8, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 62
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

  //Mask Frame 63
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};