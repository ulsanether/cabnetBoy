// ============================================================================
// retro.c - board abstraction for the CH32V003 cabnetBoy
// ============================================================================
#include "retro.h"

static uint16_t rng_state = 0xACE1;

// ------------------------------------------------------------- init --------
void retro_init(void) {
  PIN_input_AN(PIN_PAD);        // ladder input (analog, no pull)
  PIN_input_PU(PIN_ACT);        // fire button, internal pull-up
  PIN_output(PIN_BEEP);
  PIN_high(PIN_BEEP);           // PNP high-side switch -> HIGH = silent

  OLED_init();
  ADC_init();
  ADC_slow();                   // long sample window: pots/ladder are high impedance
  retro_seed(retro_pad() ^ 0x5A3C);   // a little entropy from the ADC noise
}

// ------------------------------------------------------------ ladder -------
uint16_t retro_pad(void) {
  ADC_input(PIN_PAD);           // ALWAYS re-select: reading a pot changes the channel
  return ADC_read();
}

static uint8_t ladder_to_btn(uint16_t v) {
  int16_t d;
  d = (int16_t)v - PAD_UP;
  if (d < 0) d = -d;
  if (d < PAD_TOL_UP) return BTN_UP;
  d = (int16_t)v - PAD_DOWN;
  if (d < 0) d = -d;
  if (d < PAD_TOL_DOWN) return BTN_DOWN;
  d = (int16_t)v - PAD_LEFT;
  if (d < 0) d = -d;
  if (d < PAD_TOL_LEFT) return BTN_LEFT;
  return 0;
}

uint8_t retro_buttons(void) {
  uint8_t b = ladder_to_btn(retro_pad());
  if (!PIN_read(PIN_ACT)) b |= BTN_ACT;      // active low with pull-up
  return b;
}

// Blocking read: waits until every button is released, then returns the
// first stable new press. Turn based games want exactly this.
uint8_t retro_key(void) {
  uint8_t b;
  while (retro_buttons()) DLY_ms(5);                 // wait for release
  for (;;) {
    b = retro_buttons();
    if (b) {
      DLY_ms(12);                                    // debounce
      if (retro_buttons() == b) {
        while (retro_buttons()) DLY_ms(5);           // wait for release
        return b;
      }
    }
    DLY_ms(4);
  }
}

// ------------------------------------------------------------- sound -------
// Blocking square wave. hz = 0 or ms = 0 -> silence.
void retro_beep(uint16_t hz, uint16_t ms) {
  uint16_t half;
  uint32_t n;
  if (!hz || !ms) return;
  half = (uint16_t)(500000UL / hz);
  if (half < 20) half = 20;
  n = ((uint32_t)ms * 1000UL) / (2UL * half);
  while (n--) {
    PIN_low(PIN_BEEP);
    DLY_us(half);
    PIN_high(PIN_BEEP);
    DLY_us(half);
  }
}

// ------------------------------------------------------- analog extras -----
uint8_t retro_slider(void) {
  ADC_input(PIN_SLIDER);
  return (uint8_t)(ADC_read() >> 2);
}

uint8_t retro_knob(void) {
  ADC_input(PIN_ROTATE);
  return (uint8_t)(ADC_read() >> 2);
}

// ---------------------------------------------------------------- rng ------
void retro_seed(uint16_t s) {
  if (s) rng_state = s;
}

uint16_t retro_rnd(void) {
  rng_state ^= rng_state << 7;
  rng_state ^= rng_state >> 9;
  rng_state ^= rng_state << 8;
  return rng_state;
}

// 0 .. n-1
uint8_t retro_rnd_n(uint8_t n) {
  if (!n) return 0;
  return (uint8_t)(retro_rnd() % n);
}
