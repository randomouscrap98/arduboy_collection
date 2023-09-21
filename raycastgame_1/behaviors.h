#pragma once

#include <Arduboy2.h>
#include "lib/rcsprite.h"

//#define NB_ANIMATE(name, baseframe, frames) void name(RSprite * s, Arduboy2Base * a) sprite->frame = sprite->intstate[0] + ((arduboy->frameCount >> ) & 1);

void behavior_bat(RcSprite * sprite, Arduboy2Base * arduboy)
{
    sprite->x = 4 + cos((float)arduboy->frameCount / 4) / 2;
    sprite->y = 3 + sin((float)arduboy->frameCount / 4) / 2;
    sprite->state = (sprite->state & ~(RSTATEYOFFSET)) | ((16 | uint8_t(15 * abs(sin((float)arduboy->frameCount / 11)))) << 3);
}

void behavior_animate_16(RcSprite * sprite, Arduboy2Base * arduboy)
{
    sprite->frame = sprite->intstate[0] + ((arduboy->frameCount >> 4) & (sprite->intstate[1] - 1));
}