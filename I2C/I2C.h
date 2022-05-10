// PB6 - I2C1_SCL - AF4
// PB7 - I2C1_SDA - AF4

#include "stm32f4xx.h"                  // Device header

void I2C_Init(I2C_TypeDef *I2Cx);
void I2C_Start(I2C_TypeDef *I2Cx, uint32_t DevAddress, uint8_t size, uint8_t direction);
void I2C_Stop(I2C_TypeDef *I2Cx);
void I2C_WaitLineIdle(I2C_TypeDef *I2Cx);
int8_t I2C_SendData(I2C_TypeDef *I2Cx, uint8_t SlaveAddress, uint8_t *pData, uint8_t size);
