// ============================================================================
// test.c - cabnetBoy v2 hardware self test
// ============================================================================
//
// Flash this BEFORE the game firmware on a freshly built board. It shows every
// input live so wiring mistakes are found on the bench instead of in gameplay:
//
//   row0  title
//   row1  U D L R A S      - the pressed button is shown inverted
//   row2  encoder detents + raw A/B pin state
//   row3  slider raw value
//   row4  16 cell bar of the slider
//   row5  supply voltage in mV
//   row6  slider min / max seen so far
//   row7  verdict line
//
// The verdict line catches the exact failure the old board had: a slider that
// never leaves a rail means VCC is on the wiper instead of the track end.
//
// Any button press also makes a short beep, which tests the buzzer without a
// separate menu.
// ============================================================================
#include "hw.h"
#include "gfx.h"
#include "tiles.h"

static char rows[8][VIEW_W + 1];

// ------------------------------------------------------------- formatting --
static void num(char *p, uint16_t v, uint8_t digits) {
  uint8_t i;
  for (i = 0; i < digits; i++) {
    p[digits - 1 - i] = (char)('0' + (v % 10));
    v = (uint16_t)(v / 10);
  }
}

static void signed3(char *p, int16_t v) {
  if (v < 0) { p[0] = '-'; v = (int16_t)(-v); } else { p[0] = '+'; }
  num(p + 1, (uint16_t)v, 3);
}

static void setrow(uint8_t r, const char *s) {
  uint8_t i = 0;
  while (i < VIEW_W) {
    char c = s[i];
    if (!c) break;
    if (c >= 'a' && c <= 'z') c -= 32;
    rows[r][i++] = c;
  }
  while (i < VIEW_W) rows[r][i++] = ' ';
  rows[r][VIEW_W] = 0;
}

// ------------------------------------------------------------------ state --
static uint8_t  last_btn;
static int16_t  detents;
static uint16_t smin = 1023, smax = 0;
static uint16_t ticks;                 // ~milliseconds
static uint8_t  boot_btn;              // buttons held at power on
static uint8_t  stuck_btn;

// ------------------------------------------------------------------- draw --
static void draw(void) {
  uint8_t p, i;
  uint16_t s = hw_slider_raw();
  uint8_t  b = hw_buttons();
  uint8_t  span = (uint8_t)(smax - smin);

  // --- row 0: title
  setrow(0, "CABNETBOY V2");

  // --- row 2: encoder
  rows[2][0] = 'E'; rows[2][1] = 'N'; rows[2][2] = 'C'; rows[2][3] = ' ';
  signed3(&rows[2][4], detents);
  rows[2][8] = ' '; rows[2][9] = 'A'; rows[2][10] = 'B';
  rows[2][11] = PIN_read(PIN_ENC_A) ? '1' : '0';
  rows[2][12] = PIN_read(PIN_ENC_B) ? '1' : '0';
  for (i = 13; i < VIEW_W; i++) rows[2][i] = ' ';
  rows[2][VIEW_W] = 0;

  // --- row 3: slider value
  rows[3][0] = 'S'; rows[3][1] = 'L'; rows[3][2] = 'D'; rows[3][3] = ' ';
  num(&rows[3][4], s, 4);
  for (i = 8; i < VIEW_W; i++) rows[3][i] = ' ';
  rows[3][VIEW_W] = 0;

  // --- row 5: supply
  rows[5][0] = 'V'; rows[5][1] = 'D'; rows[5][2] = 'D'; rows[5][3] = ' ';
  num(&rows[5][4], hw_vdd_mv(), 4);
  rows[5][8] = 'M'; rows[5][9] = 'V';
  for (i = 10; i < VIEW_W; i++) rows[5][i] = ' ';
  rows[5][VIEW_W] = 0;

  // --- row 6: slider range seen
  rows[6][0] = 'M'; rows[6][1] = 'I'; rows[6][2] = 'N';
  num(&rows[6][3], smin, 4);
  rows[6][7] = ' '; rows[6][8] = 'M'; rows[6][9] = 'A'; rows[6][10] = 'X';
  num(&rows[6][11], smax, 4);
  rows[6][15] = ' ';
  rows[6][VIEW_W] = 0;

  // --- row 7: verdict
  if (!hw_oled_ok())        setrow(7, "OLED NOT FOUND!");
  else if (!span && smax >= 1015) setrow(7, "SLIDER RAIL HIGH");
  else if (!span && smin <= 8)    setrow(7, "SLIDER RAIL LOW!");
  else if (stuck_btn)       setrow(7, "BTN STUCK AT BOOT");
  else if (hw_vdd_mv() < 2700)    setrow(7, "LOW BATTERY");
  else if (span >= 8)       setrow(7, "SLIDER OK");
  else                      setrow(7, "MOVE THE SLIDER");

  // --- push the pages
  for (p = 0; p < 8; p++) {
    gfx_begin(p);
    if (p == 1) {
      static const char LBL[6] = { 'U', 'D', 'L', 'R', 'A', 'S' };
      uint8_t bit;
      for (i = 0; i < VIEW_W; i++) gfx_cell(i, ' ');
      for (i = 0; i < 6; i++) {
        bit = (uint8_t)(1u << i);
        if (b & bit) gfx_cell_inv((uint8_t)(2 + i * 2), (uint8_t)LBL[i]);
        else         gfx_cell((uint8_t)(2 + i * 2), (uint8_t)LBL[i]);
      }
    } else if (p == 4) {
      gfx_bar(0, (uint8_t)(s >> 2));
    } else {
      gfx_text(0, rows[p]);
    }
    gfx_send();
  }
}

// --------------------------------------------------------------- main ------
int main(void) {
  uint8_t b, i;
  uint16_t draw_timer = 0;

  hw_init();
  boot_btn = hw_buttons();

  // If the display never answered, tell the user with the buzzer instead:
  // three short high beeps then a pause, forever, while re-probing the bus.
  while (!hw_oled_ok()) {
    for (i = 0; i < 3; i++) { hw_beep(1800, 40); DLY_ms(60); }
    DLY_ms(700);
    hw_oled_retry();
  }

  setrow(0, "CABNETBOY V2");
  setrow(7, "SELF TEST");
  draw();

  for (;;) {
    // --- fast path: poll often enough to never miss a quadrature transition
    {
      int8_t st = hw_encoder_step();
      if (st) {
        detents = (int16_t)(detents + st);
        if (detents > 999)  detents = 999;
        if (detents < -999) detents = -999;
      }
    }

    b = hw_buttons();
    if (b != last_btn) {
      last_btn = b;
      if (b) hw_beep(1500, 5);                  // buzzer check on every press
      draw_timer = 200;                         // force a redraw
    }

    {
      uint16_t s = hw_slider_raw();
      if (s < smin) smin = s;
      if (s > smax) smax = s;
    }

    // a button held down from power on is a short or a wrong footprint
    if (ticks > 1500 && !stuck_btn && boot_btn) {
      if ((hw_buttons() & boot_btn) == boot_btn) stuck_btn = boot_btn;
    }

    DLY_ms(3);
    ticks = (uint16_t)(ticks + 3);

    draw_timer = (uint16_t)(draw_timer + 3);
    if (draw_timer >= 120) {
      draw_timer = 0;
      draw();
    }
  }
}
