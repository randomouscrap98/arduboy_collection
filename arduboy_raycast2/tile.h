#pragma once

//All tiles should be 8 bytes I think

constexpr uint8_t TILESIZE = 16;
//constexpr UFixed<16,16> FTILESIZE = TILESIZE;

constexpr uint8_t wallTile[TILESIZE * 2] PROGMEM = { //0x18, 0xDB, 0xC3, 0xDB, 0x18, 0xDB, 0xC3, 0xDB };
  //Frame 0
  0x7B, 0x03, 0x7B, 0x7B, 0x7B, 0x78, 0x7B, 0x7B, 0x7B, 0x03, 0x7B, 0x7B, 0x7B, 0x78, 0x7B, 0x7B, 
  0xEF, 0x0F, 0xEF, 0xEF, 0xEF, 0xE0, 0xEF, 0xEF, 0xEF, 0x0F, 0xEF, 0xEF, 0xEF, 0xE0, 0xEF, 0xEF

  //0x7B, 0xEF, 
  //0x03, 0x0F, 
  //0x7B, 0xEF, 
  //0x7B, 0xEF, 
  //0x7B, 0xEF, 
  //0x78, 0xE0, 
  //0x7B, 0xEF, 
  //0x7B, 0xEF, 
  //0x7B, 0xEF, 
  //0x03, 0x0F, 
  //0x7B, 0xEF, 
  //0x7B, 0xEF, 
  //0x7B, 0xEF, 
  //0x78, 0xE0, 
  //0x7B, 0xEF, 
  //0x7B, 0xEF
};