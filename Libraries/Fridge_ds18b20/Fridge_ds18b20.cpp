#include "Fridge_ds18b20.h"

Fridge_ds18b20::Fridge_ds18b20(uint8_t pin)
{
	pinMode(pin, INPUT);
	bitmask = PIN_TO_BITMASK(pin);
	baseReg = PIN_TO_BASEREG(pin);
}

// Setup for all Fridge_ds18b20 sensors.
// Return:
// true - if all operations were successful
// false - when the bus is physically damaged
//       - when devices not respond
//       - when not detect any device
void Fridge_ds18b20::begin(uint8_t *dsConfig, int16_t *dsTemp, uint8_t *dsCont)
{
  uint32_t beginConversionTime;
  uint16_t i;
  uint8_t  scratchpad[9];
  
  if (*dsConfig == 0) {							// If sensor not configured
    if (!reset()) goto FAIL;						// If error, exit
    write(0xCC);                               				// Skip ROM command
    write(0x4e);							// Write scratchpad command
    write(0);								// Write Th = 0
    write(0);								// Write Tl = o
    write(0xff);							// Write quality 12 --- quality -= 9; quality <<= 5; quality |= 0b00011111;
    if (!reset()) goto FAIL;						// If error, exit
    write(0xCC);                                 			// Skip ROM
    write(0x44);                           				// Request for temperature measurements
    beginConversionTime = millis();					// Save time for request temperature
    *dsConfig = 1;							// sensor configured
    return;								// Exit
  } else {								// If sensor configured
    if (((millis() - beginConversionTime) >= 750)) {  			// Checks if devices completed the measurement
      if (!reset()) goto FAIL;						// If error, exit
      write(0xCC);           						// Skip ROM command
      write(0xbe);							// Read scratchpad command
      for (i = 0 ; i < 9 ; i++) scratchpad[i] = read();			// Read scratchpad
      *dsTemp = (int)word(scratchpad[1], scratchpad[0]) * 0.0625 * 10;	// Conversion to ºC for 12bit resolution
      if (!reset()) goto FAIL;						// If error, exit
      write(0xCC);                               			// Skip ROM command
      write(0x44);                         				// Request for temperature measurements
      beginConversionTime = millis();					// Save time for request temperature
      *dsCont = 0;							// reset failure counter      
      return;								// Exit
    }
  }
FAIL: if (*dsCont < 10) {						// if not max. consecutive fails
        *dsCont = *dsCont + 1;						// increment failure counter
      } else {								// if max. consecutive fails
        *dsConfig = 0;							// Sensor not configured or error
        *dsTemp = 9999;							// Error value for temperature readed
      }
      return;								// Exit
}


// private methods

// Perform the onewire reset function.  1=Ok to proceed, 0=no devices found or possible short to ground on bus
// First we will send a reset pulse and see if any slaves send back 
// a presence pulse. If not, then we return 0. 
// Next we check to make sure that what we saw was really a presence
// by checking to see if the bus returns to high by the pull-up. 
// If not, then their might be a short to ground so we return a 0. 
// Only if we both get a presence pulse, and we are able to see the bus return to high,
// then we return a 1 to indicate ok to proceed.
// Leaves with internal pull-up enabled

uint8_t Fridge_ds18b20::reset(void)
{
  uint8_t r;
  IO_REG_TYPE mask = bitmask;
  volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;			
  noInterrupts();							// dissable interrupts
  DIRECT_WRITE_LOW(reg, mask); 	 					// First drive the bus low to send reset pulse
  DIRECT_MODE_OUTPUT(reg, mask);	 				// drive output low
  interrupts();								// enable interrupts
  delayMicroseconds(480);			 			// Wait for salves to see the reset pulse (Trstl) 
  noInterrupts();							// dissable interrupts
  DIRECT_MODE_INPUT(reg, mask);	 					// Stop driving high 
  DIRECT_WRITE_HIGH( reg , mask ); 					// enable pull-up resistor
  delayMicroseconds(70);			 			// give the slaves a chance to pull bus low 
  r = !DIRECT_READ(reg, mask);     					// if the bus is low now, it is a presence pulse from one or more slaves
  interrupts();								// enable interrupts
  delayMicroseconds(410);          					// give slaves plenty of time to complete their presence pulse
  if (!DIRECT_READ(reg, mask)) {	 				// Check to see if the bus has failed to return to high
    return(0);					 			// If the bus is still low, then there might be a short to ground,
									// so we should not continue because actively driving high might
									// fry stuff. Could also be that the pull-up is not strong enough 
									// for connected network, in which reading will not work anyway		 
									// It would be nice to break this out to help people debug bus problems,
									// but returning an extra value could break existing code. 
  }									 	
  return r;				     				// return 0 if we saw a presence pulse or 1 if the bus stayed high after reset	
}


//
// Write a byte
// 

void Fridge_ds18b20::write(uint8_t v) {
  uint8_t bitMask;
  volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
  IO_REG_TYPE mask=bitmask;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    noInterrupts();							// Disable interrupts
    DIRECT_WRITE_LOW(reg, mask);
    DIRECT_MODE_OUTPUT(reg, mask);					// drive output low
    delayMicroseconds(65 - ((( (bitMask & v)?1:0) & 1)*50));		// bit low = 65us, bit high = 15us 
    DIRECT_WRITE_HIGH(reg, mask);					// drive output high
    interrupts();							// enable interrupts
    delayMicroseconds(((( (bitMask & v)?1:0) & 1)*50) + 5);		// Make sure output is high when slave samples 15us-60us after initial low 
  }
    noInterrupts();							// disable interrupts
    DIRECT_MODE_INPUT(baseReg, bitmask);				// if power not requested, then enable pull-up
    interrupts();							// enable interrupts
}

//
// Read a byte
//
uint8_t Fridge_ds18b20::read() {
  uint8_t bitMask;
  volatile IO_REG_TYPE *reg IO_REG_ASM = baseReg;
  IO_REG_TYPE mask=bitmask;

  uint8_t r;// = 0;

  for (bitMask = 0x01; bitMask; bitMask <<= 1) {
    noInterrupts();							// Disable interrupts
    DIRECT_WRITE_LOW(reg, mask);	
    DIRECT_MODE_OUTPUT(reg, mask);					// drive output low
    delayMicroseconds(3);						// Initiate read slot (Tint) 		
    DIRECT_MODE_INPUT(reg, mask);					// let pin float, pull up will raise
    DIRECT_WRITE_HIGH( reg, mask);					// Enable pull-up
    delayMicroseconds(10);						// Allow time for signal to rise (Trc) 
    if (DIRECT_READ(reg, mask)) r |= bitMask;
    interrupts();							// enable interrupts
    delayMicroseconds(60);						// Minimum slot time is 60us
  }
  return r;
}

