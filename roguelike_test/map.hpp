#pragma once

#include <Arduboy2.h>

constexpr uint8_t TILEEMPTY = 0;
constexpr uint8_t TILEDEFAULT = 1;
constexpr uint8_t TILEEXIT = 2;

// Each room element is 4 bytes FYI (Rect)
constexpr uint8_t ROOMSMAXDEPTH = 10;

struct Map {
  uint8_t *map;
  uint8_t width;
  uint8_t height;
};

struct PlayerSimple {
  uint8_t posX;
  uint8_t posY;
  int8_t dirX;
  int8_t dirY;
};

#define MAPT(m, x, y) m.map[(x) + (y) * m.width]

struct RoomConfig {
  uint8_t minwidth = 2;       // Each room's minimum dimension must be this.
  uint8_t doorbuffer = 1;     // Doors not generated this close to edge
  uint8_t maxwallretries = 3; // This changes the chance of "big" rooms
  uint8_t ninepool = 9;       // Larger = less likely
  uint8_t cubiclepool = 5;
  uint8_t cubiclemaxlength = 4;
  uint8_t bumppool = 5;
};

static void reset_map(Map m) {
  memset(m.map, TILEEMPTY, m.width * m.height);
  memset(m.map, TILEDEFAULT, m.width);
  memset(m.map + m.width * (m.height - 1), TILEDEFAULT, m.width);
  for (uint8_t i = 0; i < m.height; i++) {
    MAPT(m, 0, i) = TILEDEFAULT;
    MAPT(m, m.width - 1, i) = TILEDEFAULT;
  }
}

// static void fill_map(uint8_t *map, const uint8_t width, const uint8_t height)
// {
//   memset(map, TILEEMPTY, width * height);
//   memset(map, TILEDEFAULT, width);
//   memset(map + width * (height - 1), TILEDEFAULT, width);
//   for (uint8_t i = 0; i < height; i++) {
//     map[i * width] = TILEDEFAULT;
//     map[(i + 1) * width - 1] = TILEDEFAULT;
//   }
// }

// A rectangle starting at x, y and having side w, h
struct MRect {
  uint8_t x = 0;
  uint8_t y = 0;
  uint8_t w = 0;
  uint8_t h = 0;
};

struct RoomStack {
  MRect rooms[ROOMSMAXDEPTH];
  uint8_t count = 0; // 1 past the "top" of the stack
};

static bool pushRoom(RoomStack *stack, uint8_t x, uint8_t y, uint8_t w,
                     uint8_t h) {
  if (stack->count == ROOMSMAXDEPTH)
    return false;
  MRect *r = &stack->rooms[stack->count];
  r->x = x;
  r->y = y;
  r->w = w;
  r->h = h;
  stack->count += 1;
  return true;
}

static MRect popRoom(RoomStack *stack) {
  MRect result;

  if (stack->count != 0) {
    stack->count -= 1;
    memcpy(&result, &stack->rooms[stack->count], sizeof(MRect));
  }

  return result;
}

// static void set_map_ofs(uint8_t *map, uint8_t width, MRect *rect, uint8_t
// ofsx,
//                         uint8_t ofsy, uint8_t tile) {
//   map[rect->x + ofsx + (rect->y + ofsy) * width] = tile;
// }

void set_player_posdir(Map m, PlayerSimple *p, uint8_t posX, uint8_t posY) {
  p->posX = posX;
  p->posY = posY;

  if (MAPT(m, p->posX, p->posY) == TILEEMPTY) {
    p->dirX = 1;
    p->dirY = 0;
  } else {
    p->dirX = 0;
    p->dirY = 1;
  }
}

// Split rooms into smaller rooms randomly until a minimum is reached. If a room
// cannot be split but it's of certain dimensions, randomly add "interesting"
// features to it.
void genRoomsType(RoomConfig *config, Map m, PlayerSimple *p) {
  reset_map(m);
  MAPT(m, m.width - 1, m.height - 3) = TILEEMPTY;

  // Now clear out the whole inside. Might as well make the rect to represent
  // it.
  MRect crect;
  crect.x = 1;
  crect.y = 1;
  crect.w = m.width - 2;
  crect.h = m.height - 2;

  RoomStack stack;

  // Push the main room onto the stack
  pushRoom(&stack, crect.x, crect.y, crect.w, crect.h);

  // Now the main loop
  while (stack.count) {
    crect = popRoom(&stack);

    // Figure out the length of the longer side and thus the amount of places we
    // can put a wall
    uint8_t longest = max(crect.w, crect.h);
    uint8_t shortest = min(crect.w, crect.h);
    int8_t wallSpace = longest - 2 * config->minwidth;
    int8_t doorSpace = shortest - 2 * config->doorbuffer;
    uint8_t wdiv = 0;

    // Only partition the room if there's wallSpace and doorSpace. We sometimes
    // also don't touch rooms that are exactly 9 wide.
    if (wallSpace > 0 && doorSpace > 0 &&
        !(crect.w == 9 && random(config->ninepool) == 0)) {
      // We can precalc the door position. Remember, the walls are all 'shorter'
      // since they break up 'longer'
      uint8_t door = config->doorbuffer + random(doorSpace);
      uint8_t exact;

      for (uint8_t retries = 0; retries < config->maxwallretries; retries++) {
        uint8_t rnd = config->minwidth + random(wallSpace);
        if (longest == crect.w) {
          // X and Y are always INSIDE the room, not in the walls
          exact = crect.x + rnd;
          if (MAPT(m, exact, crect.y - 1) != TILEEMPTY &&
              MAPT(m, exact, crect.y + crect.h) != TILEEMPTY) {
            wdiv = exact;
            // Now draw the wall, add a door, and add the two sides to the stack
            for (uint8_t i = 0; i < shortest; i++) {
              if (i != door)
                MAPT(m, exact, crect.y + i) = TILEDEFAULT;
            }
            // Two sides are the original x,y,h + smaller width, then
            // wall+1x,y,h + smaller width
            pushRoom(&stack, crect.x, crect.y, exact - crect.x, crect.h);
            pushRoom(&stack, exact + 1, crect.y,
                     crect.w - (exact - crect.x) - 1, crect.h);
            break;
          }
        } else {
          exact = crect.y + rnd;
          if (MAPT(m, crect.x - 1, exact) != TILEEMPTY &&
              MAPT(m, crect.x + crect.w, exact) != TILEEMPTY) {
            wdiv = exact;
            // Now draw the wall, add a door, and add the two sides to the stack
            for (uint8_t i = 0; i < shortest; i++) {
              if (i != door)
                MAPT(m, crect.x + i, exact) = TILEDEFAULT;
            }
            // Two sides are original x,y,w,smaller h, then x,wall+1,w, smaller
            // h
            pushRoom(&stack, crect.x, crect.y, crect.w, exact - crect.y);
            pushRoom(&stack, crect.x, exact + 1, crect.w,
                     crect.h - (exact - crect.y) - 1);
            break;
          }
        }
      }
    }

// Yes, I am AWARE this is not a scoped macro, I'm putting it here so >> I <<
// am aware it is supposed to be scoped. It's too hard to get these out of RAM
// otherwise. The compiler refused to put any of my arrays into PROGMEM even
// though I asked, so IDK what's going on there.
#define df(ofsx, ofsy) MAPT(m, crect.x + ofsx, crect.y + ofsy) = TILEDEFAULT
    // set_map_ofs(map, width, &crect, ofsx, ofsy, TILEDEFAULT)

    // If the room was not divided, generate some things
    if (wdiv == 0) {
      // 9 and 7 are EXCEPTIONALLY rare, so they're kind of fun to stumble
      // across. The first checks are all "exact match" rooms. If not, go into
      // the "fuzzy match" rooms.
      if (crect.w == 9 && crect.h >= 12) {
        for (uint8_t i = 0; i < 5; ++i) {
          MAPT(m, crect.x + i + 2, crect.y + crect.h - 3) = TILEDEFAULT;
          MAPT(m, crect.x + i + 2, crect.y + crect.h - 7) = TILEDEFAULT;
          MAPT(m, crect.x + 2, crect.y + crect.h - i - 3) = TILEDEFAULT;
          MAPT(m, crect.x + 6, crect.y + crect.h - i - 3) = TILEDEFAULT;
        }
        MAPT(m, crect.x + 4, crect.y + crect.h - 7) = TILEEMPTY;
        MAPT(m, crect.x + 4, crect.y + crect.h - 5) = TILEEXIT;
        for (int8_t y = crect.y + crect.h - 10; y >= crect.y + 2; y -= 3) {
          MAPT(m, crect.x + 2, y) = TILEDEFAULT;
          MAPT(m, crect.x + 6, y) = TILEDEFAULT;
        }
      } else if (crect.w == 9 && crect.h == 9) {
        // if (crect.w == 9 && crect.h == 9) {
        df(2, 3);
        df(2, 2);
        df(3, 2);
        df(5, 2);
        df(6, 2);
        df(6, 3);
        df(6, 5);
        df(6, 6);
        df(5, 6);
        df(3, 6);
        df(2, 6);
        df(2, 5);
      } else if (crect.w == 7 && crect.h == 7) {
        df(2, 2);
        df(2, 4);
        df(4, 2);
        df(4, 4);
      } else if (crect.w == 6 && crect.h == 6) {
        df(1, 1);
        df(4, 1);
        df(1, 4);
        df(4, 4);
      } else if (crect.w == 5 && crect.h == 5) {
        df(2, 2);
      } else if (crect.w == 3) {
        // We can do a bit of fuzzy stuff here, it's fine. This is all
        // imprecise anyway; 3 wide rooms are common
        if (crect.h > 5 && (crect.h & 1)) {
          for (uint8_t i = crect.y + 2; i < crect.y + crect.h - 2; i += 2)
            MAPT(m, crect.x + 1, i) = TILEDEFAULT;
        } else if (crect.h > 1) {
          if (MAPT(m, crect.x + 1, crect.y - 1) != TILEEMPTY &&
              random(config->bumppool) == 0)
            MAPT(m, crect.x + 1, crect.y) = TILEDEFAULT;
        }
      } else {
        // These don't have at least one required exact width or height, so
        // they're "fuzzy"
        if (crect.w > 7 && crect.h > config->cubiclemaxlength * 2) {
          // Go around the perimeter and, with a low chance, add random length
          // walls
          for (uint8_t x = 1; x < crect.w - 1; ++x) {
            if (MAPT(m, crect.x + x, crect.y - 1) != TILEEMPTY &&
                random(config->cubiclepool) == 0) {
              for (int8_t i = random(config->cubiclemaxlength); i >= 0; --i)
                MAPT(m, crect.x + x, crect.y + i) = TILEDEFAULT;
            }
            if (MAPT(m, crect.x + x, crect.y + crect.h) != TILEEMPTY &&
                random(config->cubiclepool) == 0) {
              for (int8_t i = random(config->cubiclemaxlength); i >= 0; --i)
                MAPT(m, crect.x + x, crect.y + crect.h - 1 - i) = TILEDEFAULT;
            }
          }
        }
      }
    }
  }

  MAPT(m, m.width - 1, m.height - 3) = TILEEXIT;
  set_player_posdir(m, p, 1, 1); // posX, posY, dirX, dirY);
}

struct Type1Config {};

void gen_type_1(Type1Config *config, Map m, PlayerSimple *p) {
  memset(m.map, TILEDEFAULT, m.width * m.height);
  set_player_posdir(m, p, m.width / 2, 1); // posX, posY, dirX, dirY);
}
