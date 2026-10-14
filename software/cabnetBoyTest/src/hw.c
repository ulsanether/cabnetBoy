// ============================================================================
// hw.c - cabnetBoy v2 input layer (self test build)
// ============================================================================
#include "hw.h"

static uint8_t  enc_prev;       // previous A/B state
static int16_t  enc_raw;        // accumulated quadrature transitions
static int16_t  enc_acc;        // remainder towards the next detent
static uint8_t  oled_ok;

// ------------------------------------------------------------------ init ---
void hw_init(void) {
  PIN_input_PU(PIN_BTN_UP);
  PIN_input_PU(PIN_BTN_DOWN);
  PIN_input_PU(PIN_BTN_LEFT);
  PIN_input_PU(PIN_BTN_RIGHT);
  PIN_input_PU(PIN_BTN_ACT);
  PIN_input_PU(PIN_ENC_SW);
  PIN_input_PU(PIN_ENC_A);
  PIN_input_PU(PIN_ENC_B);
  PIN_input_AN(PIN_SLIDER);

  PIN_output(PIN_BEEP);
  PIN_high(PIN_BEEP);                 // PNP high side switch: HIGH = silent

  OLED_init();
  ADC_init();
  ADC_slow();                         // long sample window for the pot

  enc_prev = (uint8_t)((PIN_read(PIN_ENC_A) ? 2 : 0) | (PIN_read(PIN_ENC_B) ? 1 : 0));
  oled_ok = (uint8_t)(I2C_error ? 0 : 1);
}

// Re-probe the display (the self test calls this after I2C_error was raised)
uint8_t hw_oled_retry(void) {
  if (!I2C_error) return 1;
  I2C_recover();
  OLED_init();
  oled_ok = (uint8_t)(I2C_error ? 0 : 1);
  return oled_ok;
}

// --------------------------------------------------------------- buttons ---
uint8_t hw_buttons(void) {
  uint8_t b = 0;
  if (!PIN_read(PIN_BTN_UP))    b |= BTN_UP;
  if (!PIN_read(PIN_BTN_DOWN))  b |= BTN_DOWN;
  if (!PIN_read(PIN_BTN_LEFT))  b |= BTN_LEFT;
  if (!PIN_read(PIN_BTN_RIGHT)) b |= BTN_RIGHT;
  if (!PIN_read(PIN_BTN_ACT))   b |= BTN_ACT;
  if (!PIN_read(PIN_ENC_SW))    b |= BTN_SEL;
  return b;
}

uint8_t hw_buttons_debounced(void) {
  uint8_t a = hw_buttons();
  uint8_t b;
  if (!a) return 0;
  DLY_ms(8);
  b = hw_buttons();
  return (uint8_t)(a & b);
}

// --------------------------------------------------------------- encoder ---
// Gray code state table: index = (previous << 2) | current.
// A clean clockwise sequence scores -1 four times, counter clockwise +1 four
// times; an illegal double transition scores 0 so contact bounce is ignored.
static const int8_t ENC_TABLE[16] = {
   0, -1,  1,  0,
   1,  0,  0, -1,
  -1,  0,  0,  1,
   0,  1, -1,  0
};

int8_t hw_encoder_step(void) {
  uint8_t s = (uint8_t)((PIN_read(PIN_ENC_A) ? 2 : 0) | (PIN_read(PIN_ENC_B) ? 1 : 0));
  int8_t d;
  if (s == enc_prev) return 0;
  d = ENC_TABLE[((uint8_t)enc_prev << 2) | s];
  enc_prev = s;
  enc_raw += d;
  enc_acc += d;
  if (enc_acc >= ENC_DIV)  { enc_acc -= ENC_DIV; return -1; }
  if (enc_acc <= -ENC_DIV) { enc_acc += ENC_DIV; return  1; }
  return 0;
}

int16_t hw_encoder_raw(void) { return enc_raw; }

// ---------------------------------------------------------------- slider ---
uint16_t hw_slider_raw(void) {
  ADC_input(PIN_SLIDER);              // always re-select the channel
  return ADC_read();
}

uint8_t hw_slider8(void) {
  return (uint8_t)(hw_slider_raw() >> 2);
}

uint16_t hw_vdd_mv(void) {
  return ADC_read_VDD();              // uses the internal 1.2 V reference
}

uint8_t hw_oled_ok(void) { return oled_ok; }

// ----------------------------------------------------------------- sound ---
void hw_beep(uint16_t hz, uint16_t ms) {
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
