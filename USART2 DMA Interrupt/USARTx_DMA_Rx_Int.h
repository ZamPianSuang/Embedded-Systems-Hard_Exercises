#include "stm32f4xx.h"                  	// Device header

extern volatile unsigned char buffer;

void USARTx_DMA_Int_Rx_Init(USART_TypeDef * USARTx);
void USART2_GPIO_Rx_Init(void);
void DMA_Init(void);
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint8_t size);
void DMA1_Stream5_IRQHandler(void);
void delayMs(int delay);
void Led_Blinking(int c);
