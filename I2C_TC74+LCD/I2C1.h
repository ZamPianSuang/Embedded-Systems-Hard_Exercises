// PB6 - I2C1_SCL - AF4 - I2C1_Rx : DMA1 Stream5 Channel1 
// PB7 - I2C1_SDA - AF4
// Note I2C needs pull-up resistor for both SCL and SDA

#include "stm32f4xx.h"                  // Device header

extern uint8_t i2c_buff;

void I2C1_Init(void);
void DMA_Configure_I2C1_Rx(uint8_t *pRxBuffer, uint32_t size);
void I2C1_WaitLineIdle(void);
void I2C1_Start(uint32_t devAdd, uint8_t size, uint8_t direction);
void I2C1_ReceiveData(uint8_t *pRxBuffer, uint8_t SlaveAddr, uint32_t size);
// void I2C_Write(uint8_t devAdd, uint8_t memAdd, uint8_t data);
