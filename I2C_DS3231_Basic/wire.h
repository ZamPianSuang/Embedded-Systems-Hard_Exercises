#ifndef __wire__h
#define __wire__h

#include "stm32f4xx.h"                  // Device header

void I2C1_Init(void);
void I2C_readbyte(char saddr, char maddr, uint8_t *data);
void delayMs(uint32_t delay);

#endif
