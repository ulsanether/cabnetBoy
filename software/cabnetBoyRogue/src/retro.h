// ============================================================================
// retro.h - cabnetBoy v1.1 board abstraction for the CH32V003
// ============================================================================
//
// Hardware (see Schematic_RetroGame / PCB_Design_Review.md):
//
//   CH32V003A4M6  SOP16, 16K flash, 2K RAM, no hardware multiply/divide
//   SSD1306 128x64 I2C OLED on PC1 (SDA) / PC2 (SCL), 400 kHz
//   Buttons      UP / DOWN / LEFT on an ADC resistor ladder -> PC4 (ADC2)
//                ACT (BUTTON_A) digital, internal pull-up -> PD5
//   Buzzer       PA2 through a PNP high-side switch (LOW = sound)
//   Slider pot   U5  B10K   wiper -> PD6 (ADC6)   [needs the U5 hw fix]
//   Rotary pot   RV09       wiper -> PD4 (ADC7)
//
// Resistor ladder, R11 = 2k to GND, buttons pull to VCC:
//   UP 8k2 -> 0.196*VCC -> 201   DOWN 3k9 -> 0.339*VCC -> 347
//   LEFT 2k -> 0.500*VCC -> 511
// Because the ADC reference is VDD, the ratios (and these codes) do not
// depend on the battery voltage. Re-check them with software/calibrator
// if a button ever stops responding.
// ============================================================================
#pragma once

#include <stdint.h>
#include "system.h"
#include "gpio.h"
#include "oled_min.h"

// ---------------------------------------------------------------- pins -----
#define PIN_ACT     PD5      // fire / confirm button
#define PIN_BEEP    PA2      // buzzer, PNP: LOW = sound
#define PIN_PAD     PC4      // ADC2 - direction ladder
#define PIN_SLIDER  PD6      // ADC6 - U5 B10K slider
#define PIN_ROTATE  PD4      // ADC7 - RV09 rotary knob

// ------------------------------------------------------- ladder levels -----
#define PAD_IDLE    0
#define PAD_UP      201
#define PAD_DOWN    347
#define PAD_LEFT    511

#define PAD_TOL_UP    70
#define PAD_TOL_DOWN  70
#define PAD_TOL_LEFT  80

// ----------------------------------------------------------- key bits ------
#define BTN_UP    0x01
#define BTN_DOWN  0x02
#define BTN_LEFT  0x04
#define BTN_ACT   0x10

// ---------------------------------------------------------- functions ------
void     retro_init(void);
uint16_t retro_pad(void);
uint8_t  retro_buttons(void);
uint8_t  retro_key(void);           // blocking, debounced, waits for release
void     retro_beep(uint16_t hz, uint16_t ms);
uint8_t  retro_slider(void);
uint8_t  retro_knob(void);
uint16_t retro_rnd(void);
uint8_t  retro_rnd_n(uint8_t n);
void     retro_seed(uint16_t s);
