#include "stm32f4xx.h"                  	// Device header

void USARTx_DMA_Int_Rx_Init(USART_TypeDef * USARTx);
void USART2_GPIO_Rx_Init(void);
void DMA_Init(void);
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint8_t size);
void USARTx_Read(USART_TypeDef *USARTx, uint8_t *buffer, uint32_t nBytes);
void delayMs(int delay);
