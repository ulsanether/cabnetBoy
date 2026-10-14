// ============================================================================
// gfx.h - streaming 8x8 cell renderer for the SSD1306 (128x64, 1bpp)
// ============================================================================
//
// The screen is split into eight 8-pixel-tall pages. A page is exactly
// 128 bytes, so one 8x8 cell occupies one byte per column inside a page.
// Nothing is buffered in RAM except the single 128-byte page being built:
//
//     gfx_begin(page);           // clear the page buffer
//       gfx_cell(col, value);    // 16 cells per row, col 0..15
//     gfx_send();                // push 128 bytes over I2C (~3 ms at 400 kHz)
//
// Screen layout used by the game:
//     page 0..5  map viewport  16 x 6 cells
//     page 6     status line   16 characters
//     page 7     message line  16 characters
// ============================================================================
#pragma once

#include <stdint.h>

#define VIEW_W       16     // cells across
#define VIEW_H       6      // map rows (pages)
#define STATUS_PAGE  6
#define MESSAGE_PAGE 7

void gfx_begin(uint8_t page);
void gfx_cell(uint8_t col, uint8_t value);
void gfx_text(uint8_t col, const char *s);
void gfx_send(void);
void gfx_clear(void);
