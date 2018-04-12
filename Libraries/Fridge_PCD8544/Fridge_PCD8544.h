/*********************************************************************
This is a library for Monochrome Nokia 5110 LCD Displays
*********************************************************************/
#ifndef _FRIDGE_PCD8544_H
#define _FRIDGE_PCD8544_H

#include "Arduino.h"

typedef volatile uint8_t PortReg;
typedef uint8_t PortMask;

#define BLACK 1
#define WHITE 0
#define LCDWIDTH 84
#define LCDHEIGHT 48
#define PCD8544_SETYADDR 0x40
#define PCD8544_SETXADDR 0x80


class Fridge_PCD8544 : public Fridge_GFX {
 public:
  Fridge_PCD8544(int8_t SCLK, int8_t DIN, int8_t DC, int8_t RST);
  void begin(uint8_t contrast); 
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
