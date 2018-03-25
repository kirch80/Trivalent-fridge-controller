#ifndef Fridge_ds18b20_h
#define Fridge_ds18b20_h

#include <inttypes.h>
#include "Arduino.h"       // for delayMicroseconds, digitalPinToBitMask, etc

#define FALSE 0
#define TRUE  1

#define PIN_TO_BASEREG(pin)             (portInputRegister(digitalPinToPort(pin)))
#define PIN_TO_BITMASK(pin)             (digitalPinToBitMask(pin))
#define IO_REG_TYPE uint8_t
#define IO_REG_ASM asm("r30")
#define DIRECT_READ(base, mask)         (((*(base)) & (mask)) ? 1 : 0)
#define DIRECT_MODE_INPUT(base, mask)   ((*((base)+1)) &= ~(mask))
#define DIRECT_MODE_OUTPUT(base, mask)  ((*((base)+1)) |= (mask))
#define DIRECT_WRITE_LOW(base, mask)    ((*((base)+2)) &= ~(mask))
#define DIRECT_WRITE_HIGH(base, mask)   ((*((base)+2)) |= (mask))

#ifndef PROGMEM
#define PROGMEM
#endif
#ifndef pgm_read_byte
#define pgm_read_byte(addr) (*(const uint8_t *)(addr))
#endif


class Fridge_ds18b20
{
  private:
    IO_REG_TYPE bitmask;
    volatile IO_REG_TYPE *baseReg;
    uint8_t  reset(void);
    void     write(uint8_t v);
    uint8_t  read(void);

  public:
    Fridge_ds18b20(uint8_t pin);
    void     begin(uint8_t *dsConfig, int16_t *dsTemp, uint8_t *dsCont);
};

#endif
