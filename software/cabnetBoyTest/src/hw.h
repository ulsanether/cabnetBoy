// ============================================================================
// hw.h - cabnetBoy v2 pin map and input layer  (SELF TEST BUILD)
// ============================================================================
//
// This is the ONLY file you need to touch when the new PCB pin map changes.
// Every line below is a net that must exist on the board - see
// HARDWARE_SPEC.md for the wiring rules.
//
//   CH32V003A4M6 SOP16
//   pin  signal      net
//    1   PC1    I2C SDA   OLED  (+4k7 pull-up)
//    2   PC2    I2C SCL   OLED  (+4k7 pull-up)
//    3   PC3    BTN_UP    tact to GND
//    4   PC4    BTN_DOWN  tact to GND
//    5   PC6    BTN_LEFT  tact to GND
//    6   PC7    BTN_RIGHT tact to GND
//    7   PD1    SWIO      3 pin debug header  (do not load)
//    8   PD4    ENC_A     EC11 A   (+100nF to GND)
//    9   PD5    ENC_B     EC11 B   (+100nF to GND)
//   10   PD6    SLIDER    U5 B10K WIPER  (ADC6) + 100nF to GND
//   11   PD7    NRST      reset tact + 100nF to GND
//   12   PA1    ENC_SW    EC11 push switch to GND
//   13   PA2    BEEP      1k to PNP base (+10k base-emitter pull-up)
//   14   VSS    GND
//   15   VDD    VCC
//   16   PC0    BTN_ACT   tact to GND
//
// Note: the slide pot wiper MUST come here. VCC on the wiper and the ADC on a
// track end (what the old board did) makes the reading constant and shorts
// VCC to GND at one end of the travel.
// ============================================================================
#pragma once

#include <stdint.h>
#include "system.h"
#include "gpio.h"
#include "oled_min.h"

// ------------------------------------------------------------- pins --------
#define PIN_BTN_UP     PC3
#define PIN_BTN_DOWN   PC4
#define PIN_BTN_LEFT   PC6
#define PIN_BTN_RIGHT  PC7
#define PIN_BTN_ACT    PC0
#define PIN_ENC_SW     PA1
#define PIN_ENC_A      PD4
#define PIN_ENC_B      PD5
#define PIN_SLIDER     PD6     // ADC6
#define PIN_BEEP       PA2

// --------------------------------------------------------- key bits --------
#define BTN_UP    0x01
#define BTN_DOWN  0x02
#define BTN_LEFT  0x04
#define BTN_RIGHT 0x08
#define BTN_ACT   0x10
#define BTN_SEL   0x20

// EC11 variants differ: 4 quadrature transitions per detent is the common one.
// If one click moves two steps on your encoder, set this to 2.
#define ENC_DIV   4

// ---------------------------------------------------------- functions ------
void     hw_init(void);
uint8_t  hw_buttons(void);          // live level, no debounce
uint8_t  hw_buttons_debounced(void);
int8_t   hw_encoder_step(void);     // +1 / -1 per detent, 0 if nothing
int16_t  hw_encoder_raw(void);      // raw transition count (for calibration)
uint16_t hw_slider_raw(void);       // 0..1023
uint8_t  hw_slider8(void);          // 0..255
uint16_t hw_vdd_mv(void);
void     hw_beep(uint16_t hz, uint16_t ms);
uint8_t  hw_oled_ok(void);
uint8_t  hw_oled_retry(void);       // re-probe the display after an I2C error
