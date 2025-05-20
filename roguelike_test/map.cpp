#include "map.hpp"

void set_filled_map(Map m, TileConfig tiles) {
  memset(m.map, tiles.main, m.width * m.height);
  memset(m.map, tiles.perimeter, m.width);
  memset(m.map + m.width * (m.height - 1), tiles.perimeter, m.width);
  for (uint8_t i = 0; i < m.height; i++) {
    MAPT(m, 0, i) = tiles.perimeter;
    MAPT(m, m.width - 1, i) = tiles.perimeter;
  }
}

bool clear_rect_map(Map m, MRect room) {
  if (room.x > 0 && room.x + room.w <= m.width - 1 && room.y > 0 &&
      room.y + room.h <= m.height - 1) {
    for (uint8_t h = 0; h < room.h; h++) {
      for (uint8_t w = 0; w < room.w; w++) {
        MAPT(m, room.x + w, room.y + h) = TILEEMPTY;
      }
    }
    return true;
  }
  return false;
}

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

void l_to_pos(Map m, uint8_t x, uint8_t y, uint8_t ex, uint8_t ey) {
  int8_t mx = ex > x ? 1 : -1;
  int8_t my = ey > y ? 1 : -1;
  // Step towards that place in a predictable pattern
  for (; x != ex; x += mx) {
    MAPT(m, x, y) = TILEEMPTY;
  }
  for (; y != ey; y += my) {
    MAPT(m, x, y) = TILEEMPTY;
  }
}

static bool any_corner_exposed(Map m, uint8_t x, uint8_t y) {
  // Check if any corner exposed
  for (uint8_t i = 0; i < 8; i += 2) {
    uint8_t ts = 0;
    for (uint8_t c = 1; c <= 3; c++) {
      ts |=
          MAPT(m, x + pgm_read_byte(PERIMETERBUF + ((c + i) & 7)),
               y + pgm_read_byte(PERIMETERBUF + ((c + i + PERIMETERYOFS) & 7)));
    }
    if (ts == 0)
      return true;
  }
  return false;
}

void gen_type_2(Type2Config *config, Map m, MapPlayer *p) {
  set_filled_map(m, config->tiles);
  // memset(m.map, TILEDEFAULT, m.width * m.height);
  p->posX = 1 + random(m.width - 2); // m.width / 2;
  p->posY = 1 + random(m.height - 2);
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
    // l_to_pos(m, x, y, sx, sy);
    // x = sx;
    // y = sy;
    int8_t mx = sx > x ? 1 : -1;
    int8_t my = sy > y ? 1 : -1;
    // Step towards that place in a predictable pattern
    while (!(sx == x && sy == y)) {
      for (; x != sx; x += mx) {
        MAPT(m, x, y) = TILEEMPTY;
        if (random(config->turn_unlikely) == 0)
          break;
      }
      for (; y != sy; y += my) {
        MAPT(m, x, y) = TILEEMPTY;
        if (random(config->turn_unlikely) == 0)
          break;
      }
    }

    // for (; x != sx; x += mx) {
    //   MAPT(m, x, y) = TILEEMPTY;
    // }
    // for (; y != sy; y += my) {
    //   MAPT(m, x, y) = TILEEMPTY;
    // }
  }
  // Now, generate the exit along one of the alt walls
  int8_t ex, ey = 1 + random(m.width - 2);
  int8_t emx = 0, emy = 0;
  switch (random(4)) {
  case 0:
    ex = 0;
    emx = 1;
    break;
  case 1:
    ex = m.width - 1;
    emx = -1;
    break;
  case 2:
    ex = ey;
    ey = m.height - 1;
    emy = -1;
    break;
  case 3:
    ex = ey;
    ey = 0;
    emy = 1;
    break;
  }
  MAPT(m, ex, ey) = config->tiles.exit;
  ex += emx;
  ey += emy;
  // int16_t etx = ex;
  // int16_t ety = ey;
  // int16_t esdist = 9999;
  // uint8_t eex, eey;

  // l_to_pos(m, ex, ey, eex, eey);

  // Scan for the first empty slot, make a simple L to it
  for (int8_t p = 1; p < m.width / 2; p++) {
    // This wastes time rescanning but it's less code (I think)
    for (int8_t y = -p; y <= p; y++) {
      for (int8_t x = -p; x <= p; x++) {
        uint8_t tx = ex + x;
        uint8_t ty = ey + y;
        if (tx < m.width && ty < m.height && MAPT(m, tx, ty) == TILEEMPTY) {
          l_to_pos(m, ex, ey, tx, ty);
          goto ENDTYPE2EXITFIND;
        }
      }
    }
  }
ENDTYPE2EXITFIND:;

  // Scan over the whole map, doing a bunch of extra "fluff".
  for (uint8_t y = 1; y < m.height - 1; y++) {
    for (uint8_t x = 1; x < m.height - 1; x++) {
      uint8_t tile = MAPT(m, x, y);
      // if (tile == TILEEMPTY) {
      //   int16_t tx = x, ty = y;
      //   int16_t dist = DISTSQRD(etx, ety, tx, ty);
      //   if (dist < esdist) {
      //     esdist = dist;
      //     eex = x;
      //     eey = y;
      //   }
      // }
      for (uint8_t i = 0; i < config->tiles.extras_count; i++) {
        if (random(config->tiles.extras[i].unlikely) == 0) {
          switch (config->tiles.extras[i].type) {
          case TILEEXTRATYPE_NORMAL:
            if (tile != TILEEMPTY) {
              MAPT(m, x, y) = config->tiles.extras[i].tile;
            }
            break;
          case TILEEXTRATYPE_NOCORNER:
            if (tile != TILEEMPTY && !any_corner_exposed(m, x, y)) {
              MAPT(m, x, y) = config->tiles.extras[i].tile;
            }
            break;
          }
        }
      }
    }
  }

  p->cardinal = get_player_bestdir(p, m);
}
