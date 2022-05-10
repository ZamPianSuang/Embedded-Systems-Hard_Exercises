// USART1_Tx - PA9, PB6  & USART1_Rx - PA10, PB7
// UART4_Tx  - PA0, PC10 & UART4_Rx  - PA1, PC11

#include "USARTx.h"

void USARTx_Tx_Init(USART_TypeDef * USARTx)
{
	USARTx->CR1 &= ~USART_CR1_UE;								// Disable USART
	USARTx->CR1 &= ~USART_CR1_M;								// 1 Start bit, 8 Data bits, n Stop bit
	USARTx->CR2 &= ~USART_CR2_STOP;							// 1 Stop bit
	USARTx->CR1 &= ~USART_CR1_OVER8;							// Oversampling by 16
	USARTx->BRR = 0x683;											// 9600 baut rate @16 MHz
	USARTx->CR1 |= USART_CR1_TE;								// Enable Transmission 
	USARTx->CR1 |= USART_CR1_UE;								// Enable USART
}
void USARTx_Rx_Init(USART_TypeDef * USARTx)
{
	USARTx->CR1 &= ~USART_CR1_UE;								// Disable USART
	USARTx->CR1 &= ~USART_CR1_M;								// 1 Start bit, 8 Data bits, n Stop bit
	USARTx->CR2 &= ~USART_CR2_STOP;							// 1 Stop bit
	USARTx->CR1 &= ~USART_CR1_OVER8;							// Oversampling by 16
	USARTx->BRR = 0x683;											// 9600 baut rate @16 MHz
	USARTx->CR1 |= USART_CR1_RE;								// Enable Reception 
	USARTx->CR1 |= USART_CR1_UE;								// Enable USART
}
void USART2_Tx_Init(void)
{
	/* Configure PA2 as USART2_Tx */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					// Enable USART2 clock
	
	GPIOA->MODER &= ~GPIO_MODER_MODER2;
	GPIOA->MODER |= GPIO_MODER_MODE2_1;						// PA2 as Alternate mode
	GPIOA->AFR[0] |= 7 << 8;									// PA2 USART2_Tx as AF7
	
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT2;						// push-pull
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED2;				// High speed
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD2;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD2_0;						// Pull-up
	
	USARTx_Tx_Init(USART2);
}
void USART2_Rx_Init(void)
{
	/* Configure PA3 as USART2_Rx */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					// Enable USART2 clock
	
	GPIOA->MODER &= ~GPIO_MODER_MODER3;
	GPIOA->MODER |= GPIO_MODER_MODE3_1;						// PA3 as Alternate mode
	GPIOA->AFR[0] |= 7 << 12;									// PA3 USART2_Rx as AF7
	
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT3;						// push-pull
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED3;				// High speed
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3;
	GPIOA->PUPDR |= GPIO_PUPDR_PUPD3_0;						// Pull-up
	
	USARTx_Rx_Init(USART2);
}
void USARTx_Write(USART_TypeDef *USARTx, uint8_t *buffer, uint32_t nBytes)
{
	unsigned int i;
	for(i = 0; i < nBytes; i++)
	{
		while(!(USARTx->SR & USART_SR_TXE));				// Wait until data is transferred
		USARTx->DR = (buffer[i] & 0xFF);						// DR[8:0] Writing to DR clears TXE flag
	}
	while(!(USARTx->SR & USART_SR_TC));						// Wait for Transmission complete
	USARTx->SR &= ~USART_SR_TC;								// Writing 0 clear TC flag
}
void USARTx_Read(USART_TypeDef *USARTx, uint8_t *buffer, uint32_t nBytes)
{
	unsigned int i;
	for(i = 0; i < nBytes; i++)
	{
		while(!(USARTx->SR & USART_SR_RXNE));
		buffer[i] = USARTx->DR & 0xFF;
	}
}
void delayMs(int delay)	
{
	SysTick->LOAD = 16000-1; 	
	SysTick->VAL = 0;					
	SysTick->CTRL |= 0x05;		
									
	for (int i = 0; i < delay; i++)		
	{
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;			
}
