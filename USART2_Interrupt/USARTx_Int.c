// USART1_Tx - PA9, PB6  & USART1_Rx - PA10, PB7
// UART4_Tx  - PA0, PC10 & UART4_Rx  - PA1, PC11

#include "USARTx_Int.h"

volatile unsigned char buffer = 0;

void USARTx_Tx_Rx_Int_Init(USART_TypeDef * USARTx)
{
	USARTx->CR1 &= ~USART_CR1_UE;								// Disable USART
	USARTx->CR1 &= ~USART_CR1_M;								// 1 Start bit, 8 Data bits, n Stop bit
	USARTx->CR2 &= ~USART_CR2_STOP;							// 1 Stop bit
	USARTx->CR1 &= ~USART_CR1_OVER8;							// Oversampling by 16
	USARTx->BRR = 0x1117;										// 9600 baut rate @Max 45 MHz for APB1
	USARTx->CR1 |= USART_CR1_TE;								// Enable Transmission 
	USARTx->CR1 |= USART_CR1_RE;								// Enable Reception 
	USARTx->CR1 |= USART_CR1_RXNEIE;							// Enable RXNE interrupt 
	NVIC_EnableIRQ(USART2_IRQn);
	USARTx->CR1 |= USART_CR1_UE;								// Enable USART
}
void USART2_Tx_Rx_GPIO_Init(void)
{
	/* Configure PA2 as USART2_Tx and PA3 as USART2_Rx */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;										// Enable USART2 clock
	
	GPIOA->MODER |= GPIO_MODER_MODE5_0;											// PA5 for blinking
	
	GPIOA->MODER  &= ~(GPIO_MODER_MODER2 | GPIO_MODER_MODER3);
	GPIOA->MODER  |= GPIO_MODER_MODE2_1 | GPIO_MODER_MODE3_1;			// PA2 and PA3 as Alternate mode
	GPIOA->AFR[0] |= (7 << 8 |  7 << 12);										// PA2 as USART2_Tx and PA3 as USART2_Rx at AF7
	
	GPIOA->OTYPER  &= ~(GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3);				// PA2 and PA3 push-pull
	GPIOA->OSPEEDR |=  (GPIO_OSPEEDR_OSPEED2 | GPIO_OSPEEDR_OSPEED3);	// PA2 and PA3 High speed
	GPIOA->PUPDR   &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
	GPIOA->PUPDR   |=  (GPIO_PUPDR_PUPD2_0 | GPIO_PUPDR_PUPD3_0);		// PA2 and PA3 Pull-up
	
	USARTx_Tx_Rx_Int_Init(USART2);
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
void USART2_IRQHandler(void)
{
	if(USART2->SR & USART_SR_RXNE)
	{
		buffer = USART2->DR & 0xFF;
		Led_Blinking(buffer-0x30);
	}
}
void Led_Blinking(int c)
{
	for(int i = 0; i < c; i++)
	{
		GPIOA->ODR &= ~(1U << 5);
		delayMs(200);
		GPIOA->ODR |= (1 << 5);
		delayMs(200);
	}
	buffer = 0;
}
void delayMs(int delay)	
{
	SysTick->LOAD = 80000-1; 				// System Clock 80 MHz	
	SysTick->VAL = 0;					
	SysTick->CTRL |= 0x05;		
									
	for (int i = 0; i < delay; i++)		
	{
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;			
}
