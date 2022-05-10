#ifndef __I2C1_DMA__h
#define __I2C1_DMA__h

#include "stm32f4xx.h"                  // Device header

#define NULL 0

void I2C1_Init(void);
void I2C1_Rx_DMA_Init(void);
void I2C1_Tx_DMA_Init(void);
void I2C1_write(uint8_t SensorAddr, uint8_t * pWriteBuffer, uint16_t NumByteToWrite);
void I2C1_Read(uint8_t SensorAddr, uint8_t ReadAddr, uint8_t * pReadBuffer, uint16_t NumByteToRead);
void DMA1_Stream5_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);

#endif
