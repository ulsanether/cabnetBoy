// ============================================================================
// i2c_tx.c - I2C master (write only) for CH32V003, with bus timeouts
// ============================================================================
//
// Difference from the original Wagiminator driver: every wait loop has a
// timeout and an address NACK is detected, so a board without an OLED (or with
// a broken SDA/SCL joint) reports an error instead of hanging the firmware.
// The self test firmware uses I2C_error to tell the user what is wrong.
//
// 2023 by Stefan Wagner: https://github.com/wagiminator
// ============================================================================
#include "i2c_tx.h"

#define I2C_START_GENERATED     0x00010003    // BUSY, MSL, SB
#define I2C_ADDR_TRANSMITTED    0x00820003    // BUSY, MSL, ADDR, TXE
#define I2C_BYTE_TRANSMITTED    0x00840003    // BUSY, MSL, BTF, TXE
#define I2C_checkEvent(n)       (((((uint32_t)I2C1->STAR1<<16) | I2C1->STAR2) & n) == n)

volatile uint8_t I2C_error = 0;

// wait for a status combination, give up after ~60k polls (a few ms at 12 MHz)
static uint8_t i2c_wait(uint32_t mask) {
  uint32_t n = 60000;
  while (!I2C_checkEvent(mask)) {
    if (!--n) {
      I2C_error = 1;
      return 0;
    }
  }
  return 1;
}

// Init I2C
void I2C_init(void) {
  #if I2C_REMAP == 0
  // Enable GPIO port C and I2C module
  RCC->APB2PCENR |= RCC_AFIOEN | RCC_IOPCEN;
  RCC->APB1PCENR |= RCC_I2C1EN;

  // Set pin PC1 (SDA) and PC2 (SCL) to output, open-drain, 10MHz, multiplex
  GPIOC->CFGLR = (GPIOC->CFGLR & ~(((uint32_t)0b1111<<(1<<2)) | ((uint32_t)0b1111<<(2<<2))))
                               |  (((uint32_t)0b1101<<(1<<2)) | ((uint32_t)0b1101<<(2<<2)));
  #elif I2C_REMAP == 1
  RCC->APB2PCENR |= RCC_AFIOEN | RCC_IOPDEN;
  RCC->APB1PCENR |= RCC_I2C1EN;
  AFIO->PCFR1    |= 1<<1;

  GPIOD->CFGLR = (GPIOD->CFGLR & ~(((uint32_t)0b1111<<(0<<2)) | ((uint32_t)0b1111<<(1<<2))))
                               |  (((uint32_t)0b1101<<(0<<2)) | ((uint32_t)0b1101<<(1<<2)));
  #elif I2C_REMAP == 2
  RCC->APB2PCENR |= RCC_AFIOEN | RCC_IOPCEN;
  RCC->APB1PCENR |= RCC_I2C1EN;
  AFIO->PCFR1    |= 1<<22;

  GPIOC->CFGLR = (GPIOC->CFGLR & ~(((uint32_t)0b1111<<(6<<2)) | ((uint32_t)0b1111<<(5<<2))))
                               |  (((uint32_t)0b1101<<(6<<2)) | ((uint32_t)0b1101<<(5<<2)));
  #else
    #warning Wrong I2C REMAP
  #endif

  // Set I2C module clock frequency (in MHz)
  I2C1->CTLR2 = 4;

  // Set bus clock configuration
  I2C1->CKCFGR = (F_CPU / (3 * I2C_CLKRATE))
               | I2C_CKCFGR_FS;

  // Enable I2C
  I2C1->CTLR1 = I2C_CTLR1_PE;
}

// Start I2C transmission (addr must contain R/W bit)
void I2C_start(uint8_t addr) {
  uint32_t n;
  if (I2C_error) return;

  n = 60000;                                        // wait for a free bus
  while (I2C1->STAR2 & I2C_STAR2_BUSY) {
    if (!--n) { I2C_error = 1; return; }
  }

  I2C1->CTLR1 |= I2C_CTLR1_START;                   // set START condition
  if (!i2c_wait(I2C_START_GENERATED)) return;

  I2C1->DATAR = addr;                               // send slave address + R/W
  if (!i2c_wait(I2C_ADDR_TRANSMITTED)) {
    // nobody answered: clear the NACK and release the bus
    I2C1->STAR1 &= (uint16_t)~I2C_STAR1_AF;
    I2C1->CTLR1 |= I2C_CTLR1_STOP;
    I2C_error = 1;
  }
}

// Send data byte via I2C bus
void I2C_write(uint8_t data) {
  if (I2C_error) return;
  if (!i2c_wait(I2C_BYTE_TRANSMITTED)) return;
  I2C1->DATAR = data;
}

// Stop I2C transmission
void I2C_stop(void) {
  if (I2C_error) return;
  if (!i2c_wait(I2C_BYTE_TRANSMITTED)) return;
  I2C1->CTLR1 |= I2C_CTLR1_STOP;
}

// Bring the bus back after an error - the self test calls this to re-probe the
// display after the user plugs it in.
void I2C_recover(void) {
  I2C1->CTLR1 &= (uint16_t)~I2C_CTLR1_PE;
  I2C1->STAR1 &= (uint16_t)~I2C_STAR1_AF;
  I2C1->CTLR1 |= I2C_CTLR1_PE;
  I2C_error = 0;
}
