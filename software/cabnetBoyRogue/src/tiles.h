// ============================================================================
// tiles.h - 8x8 graphic tiles
// ============================================================================
// One tile = 8 bytes, one byte per pixel row, MSB = leftmost pixel.
// gfx.c transposes these into SSD1306 page format at render time.
// A screen cell holding a value < 32 means "graphic tile"; 32..95 means
// "font glyph" (see font8.h).
// ============================================================================
#pragma once

#include <stdint.h>

#define TILE_COUNT 5

#define G_VOID    0     // unexplored / out of sight
#define G_FLOOR   1     // walkable, 25% dither so the play space glows
#define G_WALL    2     // rock: left black (negative space) - the shape of the
                        // floor is what defines the dungeon. Give it a texture
                        // here later if you want visible masonry.
#define G_RUBBLE  3     // decorative debris
#define G_PLAYER  4     // the @

static const uint8_t TILES[TILE_COUNT * 8] = {
  // G_VOID
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // G_FLOOR
  0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22,
  // G_WALL
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // G_RUBBLE
  0x00, 0x14, 0x00, 0x41, 0x00, 0x28, 0x00, 0x82,
  // G_PLAYER - ring with a 2x2 centre, always obvious in a room full of dots
  0x3C, 0x42, 0x42, 0x5A, 0x5A, 0x42, 0x42, 0x3C
};
