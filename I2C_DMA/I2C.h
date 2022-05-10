// PB6 - I2C1_SCL - AF4 - I2C1_Rx : DMA1 Stream5 Channel1 
// PB7 - I2C1_SDA - AF4
// Note I2C needs pull-up resistor for both SCL and SDA

#include "stm32f4xx.h"                  // Device header

extern uint8_t i2c_buff[10];

void I2C1_Init(void);
void I2C_Write(uint8_t devAdd, uint8_t memAdd, uint8_t data);
void I2C_Read(uint8_t devAdd, uint8_t len);
void delayMs(int delay);
