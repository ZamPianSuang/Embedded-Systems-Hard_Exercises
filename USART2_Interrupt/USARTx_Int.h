#include "stm32f4xx.h"                  	// Device header

extern volatile unsigned char buffer;

void USARTx_Tx_Rx_Int_Init(USART_TypeDef * USARTx);
void USART2_Tx_Rx_GPIO_Init(void);
void USARTx_Write(USART_TypeDef *USARTx, uint8_t *buffer, uint32_t nBytes);
void USART2_IRQHandler(void);
void delayMs(int delay);
void Led_Blinking(int c);
