// ===================================================================================
// Tiny Joypad Drivers for CH32V003                                           * v1.0 *
// ===================================================================================
//
// MCU abstraction layer.
// 2023 by Stefan Wagner:   https://github.com/wagiminator

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "system.h"
#include "gpio.h"
#include "oled_min.h"

// Pin assignments
#define PIN_ACT     PD5   // pin connected to fire button
#define PIN_BEEP    PA2   // pin connected to buzzer
#define PIN_PAD     PC4   // pin conected to direction buttons
#define PIN_R_SLIDER PD6  // 슬라이더 게이지
#define PIN_R_ROTATE PD4   //로타리 조정기
#define PIN_SCL     PC2   // pin connected to OLED (I2C SCL)
#define PIN_SDA     PC1   // pin connected to OLED (I2C SDA)

// Joypad calibration values
#define JOY_N       210   // joypad UP
#define JOY_NE      259   // joypad UP + RIGHT
#define JOY_E       90    // joypad RIGHT
#define JOY_SE      388   // joypad DOWN + RIGHT
#define JOY_S       300    // joypad DOWN
#define JOY_SW      616   // joypad DOWN + LEFT
#define JOY_W       500   // joypad LEFT
#define JOY_NW      567   // JOYPAD UP + LEFT
#define JOY_DEV     20    // deviation

// Sound enable
#define JOY_SOUND   1     // 0: no sound, 1: with sound

// Game slow-down delay
#define JOY_SLOWDOWN()    DLY_us(600)

// Init driver
static inline void JOY_init(void) {  //가변 저항에 대한 초기화 추가 
  PIN_input_AN(PIN_PAD);
  PIN_input_PU(PIN_ACT);


  PIN_output(PIN_BEEP);
  PIN_high(PIN_BEEP);
  OLED_init();
  ADC_init();
  ADC_input(PIN_PAD);
  //---------------------추가된 것. 
  ADC_input(PIN_R_SLIDER);
  ADC_input(PIN_R_ROTATE);

// -------------------여기까지 

}

// OLED commands
#define JOY_OLED_init             OLED_init
#define JOY_OLED_end              I2C_stop
#define JOY_OLED_send(b)          I2C_write(b)
#define JOY_OLED_send_command(c)  OLED_send_command(c)
#define JOY_OLED_data_start(y)    {OLED_setpos(0,y);OLED_data_start();}



// Buttons
#define JOY_act_pressed()         (!PIN_read(PIN_ACT))
#define JOY_act_released()        (PIN_read(PIN_ACT))
#define JOY_pad_pressed()         (ADC_read() > 10)
#define JOY_pad_released()        (ADC_read() <= 10)
#define JOY_all_released()        (JOY_act_released() && JOY_pad_released())



//슬라이더. 
static inline uint8_t Get_PIN_R_SLIDER() {
    ADC_input(PIN_R_SLIDER);  
    uint16_t raw_value = ADC_read();  

    //  0~255 범위로 매핑
    uint8_t slider_value = (uint8_t)((raw_value * 255) / 1023);

    return slider_value;
}
//로타리 
static inline uint8_t Get_PIN_R_ROTATE() {
    ADC_input(PIN_R_ROTATE);  
    uint16_t raw_value = ADC_read();  

    //  0~255 범위로 매핑
    uint8_t rotate_value = (uint8_t)((raw_value * 255) / 1023);

    return rotate_value;
}

static inline uint8_t JOY_up_pressed(void) {
 uint16_t val = ADC_read();



//val값을 받아서 어떻게 쓸건지 확인을 해야함. 
 return(   ((val > JOY_N  - JOY_DEV) && (val < JOY_N  + JOY_DEV))
         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV)) );
}



static inline uint8_t JOY_down_pressed(void) {
 uint16_t val = ADC_read();
 return(   ((val > JOY_S  - JOY_DEV) && (val < JOY_S  + JOY_DEV))
         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV))
         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

static inline uint8_t JOY_left_pressed(void) {
 uint16_t val = ADC_read();
 return(   ((val > JOY_W  - JOY_DEV) && (val < JOY_W  + JOY_DEV))
         | ((val > JOY_NW - JOY_DEV) && (val < JOY_NW + JOY_DEV))
         | ((val > JOY_SW - JOY_DEV) && (val < JOY_SW + JOY_DEV)) );
}

static inline uint8_t JOY_right_pressed(void) {
 uint16_t val = ADC_read();
 return(   ((val > JOY_E  - JOY_DEV) && (val < JOY_E  + JOY_DEV))
         | ((val > JOY_NE - JOY_DEV) && (val < JOY_NE + JOY_DEV))
         | ((val > JOY_SE - JOY_DEV) && (val < JOY_SE + JOY_DEV)) );
}

// Buzzer
void JOY_sound(uint8_t freq, uint8_t dur) {
 

  while(dur--) {
    #if JOY_SOUND == 1
    if(freq) PIN_low(PIN_BEEP);
    #endif
    DLY_us(255 - freq);
    PIN_high(PIN_BEEP);
    DLY_us(255 - freq);
  }
}

// Pseudo random number generator
uint16_t rnval = 0xACE1;
uint16_t JOY_random(void) {
  rnval = (rnval >> 0x01) ^ (-(rnval & 0x01) & 0xB400);
  return rnval;
}

// Delays
#define JOY_DLY_ms    DLY_ms
#define JOY_DLY_us    DLY_us

// Additional Defines
#define abs(n) ((n>=0)?(n):(-(n)))

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

#ifdef __cplusplus
};
#endif
