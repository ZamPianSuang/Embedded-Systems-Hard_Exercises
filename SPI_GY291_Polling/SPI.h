#ifndef __SPI__
#define __SPI__

#include "stm32f4xx.h"                  									// Device header

#define ADXL345_CS_LOW				GPIOA->ODR &= ~(1U << 4)			// CS/NSS -> PA4
#define ADXL345_CS_HIGH				GPIOA->ODR |= (1U << 4)				// CS/NSS -> PA4

void SPI1_Init(void);
void SPI1_Write(uint8_t *txBuffer, uint8_t *rxBuffer, int size);
void SPI1_Read(uint8_t *rxBuffer, int size);
void GYRO_IO_Write(uint8_t *pBuffer, uint8_t WriteAddr, uint8_t size);
void GYRO_IO_Read(uint8_t *pBuffer, uint8_t ReadAddr, uint8_t size);
void delayMs(int delay);

#endif
