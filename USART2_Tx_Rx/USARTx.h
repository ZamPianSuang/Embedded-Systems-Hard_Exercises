#include "stm32f4xx.h"                  	// Device header

void USARTx_Tx_Init(USART_TypeDef * USARTx);
void USARTx_Rx_Init(USART_TypeDef * USARTx);
void USART2_Tx_Init(void);
void USART2_Rx_Init(void);
void USARTx_Write(USART_TypeDef *USARTx, uint8_t *buffer, uint32_t nBytes);
void USARTx_Read(USART_TypeDef *USARTx, uint8_t *buffer, uint32_t nBytes);
void delayMs(int delay);
