#include "map.hpp"

void set_empty_map(Map m) {
  memset(m.map, TILEEMPTY, m.width * m.height);
  memset(m.map, TILEDEFAULT, m.width);
  memset(m.map + m.width * (m.height - 1), TILEDEFAULT, m.width);
  for (uint8_t i = 0; i < m.height; i++) {
    MAPT(m, 0, i) = TILEDEFAULT;
    MAPT(m, m.width - 1, i) = TILEDEFAULT;
  }
}

bool clear_rect_map(Map m, MRect room) {
  if (room.x > 0 && room.x + room.w < m.width - 1 && room.y > 0 &&
      room.y + room.h < m.height - 1) {
    for (uint8_t h = 0; h < room.h; h++) {
      for (uint8_t w = 0; w < room.w; w++) {
        MAPT(m, room.x + w, room.y + h) = TILEEMPTY;
      }
    }
    return true;
  }
  return false;
}

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

// void genRoomsType(RoomConfig *config, Map m, MapPlayer *p) {
//   set_empty_map(m);
//   MAPT(m, m.width - 1, m.height - 3) = TILEEMPTY;
//
//   // Now clear out the whole inside. Might as well make the rect to represent
//   // it.
//   MRect crect;
//   crect.x = 1;
//   crect.y = 1;
//   crect.w = m.width - 2;
//   crect.h = m.height - 2;
//
//   RoomStack stack;
//
//   // Push the main room onto the stack
//   pushRoom(&stack, crect.x, crect.y, crect.w, crect.h);
//
//   // Now the main loop
//   while (stack.count) {
//     crect = popRoom(&stack);
//
//     // Figure out the length of the longer side and thus the amount of places
//     we
//     // can put a wall
//     uint8_t longest = max(crect.w, crect.h);
//     uint8_t shortest = min(crect.w, crect.h);
//     int8_t wallSpace = longest - 2 * config->minwidth;
//     int8_t doorSpace = shortest - 2 * config->doorbuffer;
//     uint8_t wdiv = 0;
//
//     // Only partition the room if there's wallSpace and doorSpace. We
//     sometimes
//     // also don't touch rooms that are exactly 9 wide.
//     if (wallSpace > 0 && doorSpace > 0 &&
//         !(crect.w == 9 && random(config->ninepool) == 0)) {
//       // We can precalc the door position. Remember, the walls are all
//       'shorter'
//       // since they break up 'longer'
//       uint8_t door = config->doorbuffer + random(doorSpace);
//       uint8_t exact;
//
//       for (uint8_t retries = 0; retries < config->maxwallretries; retries++)
//       {
//         uint8_t rnd = config->minwidth + random(wallSpace);
//         if (longest == crect.w) {
//           // X and Y are always INSIDE the room, not in the walls
//           exact = crect.x + rnd;
//           if (MAPT(m, exact, crect.y - 1) != TILEEMPTY &&
//               MAPT(m, exact, crect.y + crect.h) != TILEEMPTY) {
//             wdiv = exact;
//             // Now draw the wall, add a door, and add the two sides to the
//             stack for (uint8_t i = 0; i < shortest; i++) {
//               if (i != door)
//                 MAPT(m, exact, crect.y + i) = TILEDEFAULT;
//             }
//             // Two sides are the original x,y,h + smaller width, then
//             // wall+1x,y,h + smaller width
//             pushRoom(&stack, crect.x, crect.y, exact - crect.x, crect.h);
//             pushRoom(&stack, exact + 1, crect.y,
//                      crect.w - (exact - crect.x) - 1, crect.h);
//             break;
//           }
//         } else {
//           exact = crect.y + rnd;
//           if (MAPT(m, crect.x - 1, exact) != TILEEMPTY &&
//               MAPT(m, crect.x + crect.w, exact) != TILEEMPTY) {
//             wdiv = exact;
//             // Now draw the wall, add a door, and add the two sides to the
//             stack for (uint8_t i = 0; i < shortest; i++) {
//               if (i != door)
//                 MAPT(m, crect.x + i, exact) = TILEDEFAULT;
//             }
//             // Two sides are original x,y,w,smaller h, then x,wall+1,w,
//             smaller
//             // h
//             pushRoom(&stack, crect.x, crect.y, crect.w, exact - crect.y);
//             pushRoom(&stack, crect.x, exact + 1, crect.w,
//                      crect.h - (exact - crect.y) - 1);
//             break;
//           }
//         }
//       }
//     }
//
// // Yes, I am AWARE this is not a scoped macro, I'm putting it here so >> I <<
// // am aware it is supposed to be scoped. It's too hard to get these out of
// RAM
// // otherwise. The compiler refused to put any of my arrays into PROGMEM even
// // though I asked, so IDK what's going on there.
// #define df(ofsx, ofsy) MAPT(m, crect.x + ofsx, crect.y + ofsy) = TILEDEFAULT
//     // set_map_ofs(map, width, &crect, ofsx, ofsy, TILEDEFAULT)
//
//     // If the room was not divided, generate some things
//     if (wdiv == 0) {
//       // 9 and 7 are EXCEPTIONALLY rare, so they're kind of fun to stumble
//       // across. The first checks are all "exact match" rooms. If not, go
//       into
//       // the "fuzzy match" rooms.
//       if (crect.w == 9 && crect.h >= 12) {
//         for (uint8_t i = 0; i < 5; ++i) {
//           MAPT(m, crect.x + i + 2, crect.y + crect.h - 3) = TILEDEFAULT;
//           MAPT(m, crect.x + i + 2, crect.y + crect.h - 7) = TILEDEFAULT;
//           MAPT(m, crect.x + 2, crect.y + crect.h - i - 3) = TILEDEFAULT;
//           MAPT(m, crect.x + 6, crect.y + crect.h - i - 3) = TILEDEFAULT;
//         }
//         MAPT(m, crect.x + 4, crect.y + crect.h - 7) = TILEEMPTY;
//         MAPT(m, crect.x + 4, crect.y + crect.h - 5) = TILEEXIT;
//         for (int8_t y = crect.y + crect.h - 10; y >= crect.y + 2; y -= 3) {
//           MAPT(m, crect.x + 2, y) = TILEDEFAULT;
//           MAPT(m, crect.x + 6, y) = TILEDEFAULT;
//         }
//       } else if (crect.w == 9 && crect.h == 9) {
//         // if (crect.w == 9 && crect.h == 9) {
//         df(2, 3);
//         df(2, 2);
//         df(3, 2);
//         df(5, 2);
//         df(6, 2);
//         df(6, 3);
//         df(6, 5);
//         df(6, 6);
//         df(5, 6);
//         df(3, 6);
//         df(2, 6);
//         df(2, 5);
//       } else if (crect.w == 7 && crect.h == 7) {
//         df(2, 2);
//         df(2, 4);
//         df(4, 2);
//         df(4, 4);
//       } else if (crect.w == 6 && crect.h == 6) {
//         df(1, 1);
//         df(4, 1);
//         df(1, 4);
//         df(4, 4);
//       } else if (crect.w == 5 && crect.h == 5) {
//         df(2, 2);
//       } else if (crect.w == 3) {
//         // We can do a bit of fuzzy stuff here, it's fine. This is all
//         // imprecise anyway; 3 wide rooms are common
//         if (crect.h > 5 && (crect.h & 1)) {
//           for (uint8_t i = crect.y + 2; i < crect.y + crect.h - 2; i += 2)
//             MAPT(m, crect.x + 1, i) = TILEDEFAULT;
//         } else if (crect.h > 1) {
//           if (MAPT(m, crect.x + 1, crect.y - 1) != TILEEMPTY &&
//               random(config->bumppool) == 0)
//             MAPT(m, crect.x + 1, crect.y) = TILEDEFAULT;
//         }
//       } else {
//         // These don't have at least one required exact width or height, so
//         // they're "fuzzy"
//         if (crect.w > 7 && crect.h > config->cubiclemaxlength * 2) {
//           // Go around the perimeter and, with a low chance, add random
//           length
//           // walls
//           for (uint8_t x = 1; x < crect.w - 1; ++x) {
//             if (MAPT(m, crect.x + x, crect.y - 1) != TILEEMPTY &&
//                 random(config->cubiclepool) == 0) {
//               for (int8_t i = random(config->cubiclemaxlength); i >= 0; --i)
//                 MAPT(m, crect.x + x, crect.y + i) = TILEDEFAULT;
//             }
//             if (MAPT(m, crect.x + x, crect.y + crect.h) != TILEEMPTY &&
//                 random(config->cubiclepool) == 0) {
//               for (int8_t i = random(config->cubiclemaxlength); i >= 0; --i)
//                 MAPT(m, crect.x + x, crect.y + crect.h - 1 - i) =
//                 TILEDEFAULT;
//             }
//           }
//         }
//       }
//     }
//   }
//
//   MAPT(m, m.width - 1, m.height - 3) = TILEEXIT;
//   //set_player_posdir(m, p, 1, 1); // posX, posY, dirX, dirY);
// }

// filled true = check for all filled instead of empty

// Check if there's any empty spots in given rect in map, or if the map
// exceeds the bounds
// static bool map_area_overlaps(Map m, MRect r) { //, bool filled) {
//   for (uint8_t y = 0; y < r.h; y++) {
//     for (uint8_t x = 0; x < r.w; x++) {
//       uint8_t rx = x + r.x, ry = y + r.y;
//       if (rx < 1 || ry < 1 || rx >= m.width - 1 || ry >= m.height - 1) {
//         return true;
//       }
//       if (MAPT(m, rx, ry) == TILEEMPTY) {
//         return true;
//       }
//     }
//   }
//   return false;
// }

static bool map_area_overlaps_buffer(Map m, MRect r) { //, bool filled) {
  for (uint8_t y = 0; y < r.h + 2; y++) {
    for (uint8_t x = 0; x < r.w + 2; x++) {
      uint8_t rx = x + r.x - 1, ry = y + r.y - 1;
      if (rx > m.width - 1 || ry > m.height - 1) {
        return true;
      }
      if (MAPT(m, rx, ry) == TILEEMPTY) {
        return true;
      }
    }
  }
  return false;
}

static bool move_dir_ok(Map m, uint8_t x, uint8_t y, int8_t dx, int8_t dy) {
  uint8_t cx = x + dx;
  uint8_t cy = y + dy;
  return ((cx > 0) && (cy > 0) && (cx < m.width - 1) && (cy < m.height - 1) &&
          (MAPT(m, cx, cy) != TILEEMPTY));
}

//  maybe meander about, spawning rooms at intervals? is ithere even enough
//  room for that?
void gen_type_1(Type1Config *config, Map m, MapPlayer *p) {
  memset(m.map, TILEDEFAULT, m.width * m.height);
  p->posX = m.width / 2;
  p->posY = 1;
  int8_t dx = 0, dy = 1;
  //  start walking through, setting current position to empty, deciding on a
  //  room, then figuring out if you need to change directions.
  uint8_t x = p->posX, y = p->posY;
  while (1) {
    MAPT(m, x, y) = TILEEMPTY;
    // if (random(config->room_pool) == 0) {
    //   for (uint8_t rt = 0; rt < config->room_retries; rt++) {
    //     uint8_t rx = x, ry = y;
    //     MRect room;
    //     room.w =
    //         config->room_min + random(1 + config->room_max -
    //         config->room_min);
    //     room.h =
    //         config->room_min + random(1 + config->room_max -
    //         config->room_min);
    //     int8_t mod = random(2) ? -1 : 1;
    //     bool valid = false;
    //     if (dy == 0) { // Moving horizontally, spawn vertically
    //       ry += 2 * mod;
    //       room.y = ry + (mod < 0) ? 1 - room.h : 0;
    //       for (uint8_t i = 0; i < room.w; i++) {
    //         room.x = x - i;
    //         // pick the first that works
    //         if (!map_area_overlaps_buffer(m, room)) {
    //           valid = true;
    //           break;
    //         }
    //       }
    //     } else { // moving vertically, spawn horizontally
    //       rx += 2 * mod;
    //       room.x = rx + (mod < 0) ? 1 - room.w : 0;
    //       for (uint8_t i = 0; i < room.h; i++) {
    //         room.y = y - i;
    //         // pick the first that works
    //         if (!map_area_overlaps_buffer(m, room)) {
    //           valid = true;
    //           break;
    //         }
    //       }
    //     }
    //     if (!valid) {
    //       continue;
    //     }
    //     // I guess just make the room?
    //     for (uint8_t h = 0; h < room.h; h++) {
    //       for (uint8_t w = 0; w < room.w; w++) {
    //         MAPT(m, room.x + w, room.y + h) = TILEEMPTY;
    //       }
    //     }
    //     // This clears out the hallway to the room
    //     MAPT(m, (rx + x) / 2, (ry + y) / 2) = TILEEMPTY;
    //   }
    // }
    // Change direction randomly
    if (!move_dir_ok(m, x, y, dx, dy) || random(config->hw_cdpool) == 0) {
      uint8_t bd = random(4);
      for (uint8_t bdi = 0; bdi < 4; bdi++) {
        // Check all directions; if they all fail, we can't continue (?)
        cardinal_to_dir((bd + bdi) & 3, &dx, &dy);
        // set_player_dir(p, (bd + bdi) & 3);
        if (move_dir_ok(m, x, y, dx, dy)) {
          goto FOUNDDIR;
        }
      }
      break;
    FOUNDDIR:;
    }
    // For moving by 2
    // MAPT(m, x + dx / 2, y + dy / 2) = TILEEMPTY;
    x += dx;
    y += dy;
    // For early stops
    // if (random(config->hw_stoppool) == 0) {
    //   break;
    // }
  }

  p->cardinal = get_player_bestdir(p, m);
  // set_player_dir(p, get_player_bestdir(p, m));
}

void gen_type_2(Type2Config *config, Map m, MapPlayer *p) {
  memset(m.map, TILEDEFAULT, m.width * m.height);
  p->posX = 1 + random(m.width - 2); // m.width / 2;
  p->posY = 1;
  //  start walking through, setting current position to empty, deciding on a
  //  room, then figuring out if you need to change directions.
  uint8_t x = p->posX, y = p->posY;
  for (uint8_t stop = 0; stop < config->stops; stop++) {
    // Pick a place to stop
    uint8_t sx = 1 + random(m.width - 2);
    uint8_t sy = 1 + random(m.height - 2);
    if (random(config->room_unlikely) == 0) {
      // Try to generate a room.
      MRect room = {
          .x = sx,
          .y = sy,
          .w = RANDROOMDIM(config),
          .h = RANDROOMDIM(config),
      };
      clear_rect_map(m, room);
    }
    int8_t mx = sx > x ? 1 : -1;
    int8_t my = sy > y ? 1 : -1;
    // Step towards that place in a predictable pattern
    // while (sx != x && sy != y) {
    //   for (; x != sx; x += mx) {
    //     MAPT(m, x, y) = TILEEMPTY;
    //     if (random(config->turn_unlikely) == 0)
    //       break;
    //   }
    //   for (; y != sy; y += my) {
    //     MAPT(m, x, y) = TILEEMPTY;
    //     if (random(config->turn_unlikely) == 0)
    //       break;
    //   }
    // }
    for (; x != sx; x += mx) {
      MAPT(m, x, y) = TILEEMPTY;
    }
    for (; y != sy; y += my) {
      MAPT(m, x, y) = TILEEMPTY;
    }
  }
  p->cardinal = get_player_bestdir(p, m);
}
