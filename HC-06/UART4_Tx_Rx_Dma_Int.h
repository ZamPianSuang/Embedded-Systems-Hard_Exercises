// UART4_Tx - PC10 - AF8 - DMA1 Stream4 Channel4
// UART4_Rx - PC11 - AF8 - DMA1 Stream2 Channel4

#include "stm32f4xx.h"                  // Device header

extern volatile unsigned char buffer;

void PC10_PC11_UART4_Init(void);
void UART4_Tx_Rx_DMA_Init(void);
void DMA_Init(void);
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint8_t size);
void DMA1_Stream2_IRQHandler(void);
void delayMs(int delay);
void Led_Blinking(int c);
