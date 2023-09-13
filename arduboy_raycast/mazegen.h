#pragma once

#include "utils.h"

struct MazeType
{
    char name[4];
    void * func;
};


constexpr uint8_t MAZETYPECOUNT = 3;

constexpr MazeType MAZETYPES[MAZETYPECOUNT] = {
    { "MAZ", NULL },
    { "RMS", NULL },
    { "CEL", NULL }
};
