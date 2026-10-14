// ============================================================================
// tiles.h - 8x8 tiles for the self-test screens
// ============================================================================
// One tile = 8 bytes, one byte per pixel row, MSB = leftmost.
// gfx.c transposes them into SSD1306 page format.
// ============================================================================
#pragma once

#include <stdint.h>

#define TILE_COUNT 4

#define G_VOID  0     // empty (used for the unfilled part of a bar)
#define G_FULL  1     // solid block
#define G_HALF  2     // top half - finer bar steps
#define G_BOX   3     // hollow box - button indicator frame

static const uint8_t TILES[TILE_COUNT * 8] = {
  // G_VOID
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  // G_FULL
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
  // G_HALF
  0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
  // G_BOX
  0x00, 0x7E, 0x42, 0x42, 0x42, 0x42, 0x7E, 0x00
};
