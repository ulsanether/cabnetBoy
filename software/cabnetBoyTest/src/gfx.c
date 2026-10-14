// ============================================================================
// gfx.c - streaming 8x8 cell renderer for the SSD1306
// ============================================================================
#include "gfx.h"
#include "hw.h"
#include "font8.h"
#include "tiles.h"

static uint8_t page_buf[128];    // one SSD1306 page
static uint8_t cur_page;

// ---------------------------------------------------------------- begin ----
void gfx_begin(uint8_t page) {
  uint8_t i;
  cur_page = page;
  for (i = 0; i < 128; i++) page_buf[i] = 0;
}

// ------------------------------------------------------------ glyph ptr ----
static const uint8_t *glyph_of(uint8_t value) {
  if (value < 32) {
    if (value >= TILE_COUNT) value = G_VOID;
    return &TILES[value * 8];
  }
  if (value > 95) value = 32;                 // anything above '_' -> blank
  return &FONT8[(value - 32) * 8];
}

// ----------------------------------------------------------- put a cell ----
// Transpose one 8x8 row-major bitmap into 8 SSD1306 column bytes.
// 16 cells x 64 bit tests = one page, a few hundred cycles - far cheaper
// than keeping a 1 KB frame buffer in the 2 KB of SRAM.
void gfx_cell(uint8_t col, uint8_t value) {
  const uint8_t *p = glyph_of(value);
  uint8_t *o = &page_buf[col << 3];
  uint8_t py, b, bit;

  o[0] = o[1] = o[2] = o[3] = o[4] = o[5] = o[6] = o[7] = 0;
  for (py = 0; py < 8; py++) {
    b = p[py];
    if (!b) continue;
    bit = (uint8_t)(1u << py);
    if (b & 0x80) o[0] |= bit;
    if (b & 0x40) o[1] |= bit;
    if (b & 0x20) o[2] |= bit;
    if (b & 0x10) o[3] |= bit;
    if (b & 0x08) o[4] |= bit;
    if (b & 0x04) o[5] |= bit;
    if (b & 0x02) o[6] |= bit;
    if (b & 0x01) o[7] |= bit;
  }
}

// ------------------------------------------------------------- put text ----
// ------------------------------------------------------- inverted cell -----
// Same glyph with every pixel flipped - used to highlight pressed buttons.
void gfx_cell_inv(uint8_t col, uint8_t value) {
  uint8_t *o = &page_buf[col << 3];
  uint8_t i;
  gfx_cell(col, value);
  for (i = 0; i < 8; i++) o[i] = (uint8_t)~o[i];
}

// ------------------------------------------------------------------ bar ----
// level 0..255 spread over 8 cells; the last partial cell uses G_HALF so the
// resolution is fine enough to see ADC jitter.
void gfx_bar(uint8_t col, uint8_t level) {
  uint8_t filled = (uint8_t)(((uint16_t)level * 16u) >> 8);   // 0..15 half cells
  uint8_t i;
  for (i = 0; i < 8; i++) {
    if (filled >= 2)      { gfx_cell((uint8_t)(col + i), G_FULL); filled -= 2; }
    else if (filled == 1) { gfx_cell((uint8_t)(col + i), G_HALF); filled = 0; }
    else                  { gfx_cell((uint8_t)(col + i), G_VOID); }
  }
}

void gfx_text(uint8_t col, const char *s) {
  while (*s && col < VIEW_W) {
    uint8_t c = (uint8_t)*s++;
    if (c >= 'a' && c <= 'z') c -= 32;        // fold to upper case
    gfx_cell(col++, c);
  }
}

void gfx_text_inv(uint8_t col, const char *s) {
  while (*s && col < VIEW_W) {
    uint8_t c = (uint8_t)*s++;
    if (c >= 'a' && c <= 'z') c -= 32;
    gfx_cell_inv(col++, c);
  }
}

// ---------------------------------------------------------------- send -----
// If the I2C bus is dead (no OLED fitted) the transfers are skipped instead of
// spinning on a timeout 1024 times per frame.
void gfx_send(void) {
  uint8_t i;
  if (I2C_error) return;
  OLED_setpos(0, cur_page);
  OLED_data_start();
  for (i = 0; i < 128; i++) I2C_write(page_buf[i]);
  I2C_stop();
}

// --------------------------------------------------------------- clear -----
void gfx_clear(void) {
  uint8_t p;
  for (p = 0; p < 8; p++) {
    gfx_begin(p);
    gfx_send();
  }
}
