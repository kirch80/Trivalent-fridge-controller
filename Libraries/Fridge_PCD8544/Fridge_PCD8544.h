/*********************************************************************
This is a library for Monochrome Nokia 5110 LCD Displays
*********************************************************************/
#ifndef _FRIDGE_PCD8544_H
#define _FRIDGE_PCD8544_H

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
#endif

//#include <SPI.h>

typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;

#define BLACK 1
#define WHITE 0

#define LCDWIDTH 84
#define LCDHEIGHT 48

#define PCD8544_POWERDOWN 0x04
#define PCD8544_ENTRYMODE 0x02
#define PCD8544_EXTENDEDINSTRUCTION 0x01

#define PCD8544_DISPLAYBLANK 0x0
#define PCD8544_DISPLAYNORMAL 0x4
#define PCD8544_DISPLAYALLON 0x1
#define PCD8544_DISPLAYINVERTED 0x5

// H = 0
#define PCD8544_FUNCTIONSET 0x20
#define PCD8544_DISPLAYCONTROL 0x08
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80

// H = 1
#define PCD8544_SETTEMP 0x04
#define PCD8544_SETBIAS 0x10
#define PCD8544_SETVOP 0x80

#define PCD8544_SPI_CLOCK_DIV SPI_CLOCK_DIV4

class Fridge_PCD8544 : public Fridge_GFX {
 public:
  Fridge_PCD8544(int8_t SCLK, int8_t DIN, int8_t DC, int8_t RST);
  void begin(uint8_t contrast, uint8_t bias = 0x04); //40 y 0x04
  void command(uint8_t c);
  void data(uint8_t c);
  void setContrast(uint8_t val);
  void clearDisplay(void);
  void display();
  void drawPixel(int16_t x, int16_t y, uint16_t color);

 private:
  int8_t _din, _sclk, _dc, _rst;
  volatile PortReg  *mosiport, *clkport;
  PortMask mosipinmask, clkpinmask;
  void spiWrite(uint8_t c);
};

#endif
