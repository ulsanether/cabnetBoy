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

#define TILE_COUNT 8

#define G_VOID    0     // unexplored / out of sight
#define G_FLOOR   1     // walkable, 25% dither so the play space glows
#define G_WALL    2     // rock: left black (negative space) - the shape of the
                        // floor is what defines the dungeon. Give it a texture
                        // here later if you want visible masonry.
#define G_RUBBLE  3     // decorative debris

// The @ is drawn facing its four directions. The knob controls which one is
// used, so the pointer tick is the player's only feedback for where they aim.
#define G_PL_N    4
#define G_PL_E    5
#define G_PL_S    6
#define G_PL_W    7

static const uint8_t TILES[TILE_COUNT * 8] = {
  // G_VOID
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // G_FLOOR
  0x88, 0x22, 0x88, 0x22, 0x88, 0x22, 0x88, 0x22,
  // G_WALL
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // G_RUBBLE
  0x00, 0x14, 0x00, 0x41, 0x00, 0x28, 0x00, 0x82,
  // G_PL_N : ring + tick above
  0x18, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x00,
  // G_PL_E : ring + tick right
  0x00, 0x3C, 0x42, 0x43, 0x43, 0x42, 0x3C, 0x00,
  // G_PL_S : ring + tick below
  0x00, 0x3C, 0x42, 0x42, 0x42, 0x42, 0x3C, 0x18,
  // G_PL_W : ring + tick left
  0x00, 0x3C, 0x42, 0xC2, 0xC2, 0x42, 0x3C, 0x00
};
