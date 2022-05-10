// USART2_Rx - PA3
// USART2_Rx - DMA1 Channel4 Stream5

#include "USARTx_DMA_Rx_Int.h"

volatile unsigned char buffer;

void USARTx_DMA_Int_Rx_Init(USART_TypeDef * USARTx)
{
	USARTx->CR1 &= ~USART_CR1_UE;								// Disable USART
	USARTx->CR1 &= ~USART_CR1_M;								// 1 Start bit, 8 Data bits, n Stop bit
	USARTx->CR2 &= ~USART_CR2_STOP;							// 1 Stop bit
	USARTx->CR1 &= ~USART_CR1_OVER8;							// Oversampling by 16
	USARTx->BRR = 0x683;											// 9600 baut rate @16 MHz
	USARTx->CR1 |= USART_CR1_RE;								// Enable Reception 
	USARTx->CR1 |= USART_CR1_UE;								// Enable USART
	
	/****************************** DMA ******************************/
	USARTx->CR3 |= USART_CR3_DMAR;							// Enable DMA receiver
}
void USART2_GPIO_Rx_Init(void)
{
	/* Configure PA3 as USART2_Rx */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN;					// Enable USART2 clock
	
	GPIOA->MODER |= GPIO_MODER_MODE5_0;						// PA5 for blinking function
	
	GPIOA->MODER &= ~GPIO_MODER_MODER3;
	GPIOA->MODER |= GPIO_MODER_MODE3_1;						// PA3 as Alternate mode
	GPIOA->AFR[0] |= 7 << 12;									// PA3 USART2_Rx as AF7
	
	GPIOA->OTYPER &= ~GPIO_OTYPER_OT3;						// push-pull
	GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED3;				// High speed
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD3;						// No pull-up/pull-down
	
	USARTx_DMA_Int_Rx_Init(USART2);
}
void DMA_Init(void)
{
	/* USART2_Rx - DMA1, Channel4, Stream5 on F446RE */
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;					// Enable clock for DMA1
	
	DMA1_Stream5->CR &= ~(3U << 6);						// Peripherial to Memory
	DMA1_Stream5->CR |= 1 << 8;							// Circular mode
	DMA1_Stream5->CR &= ~(1U << 9);						// Disable peripherial increment mode
	DMA1_Stream5->CR |= 1 << 10;							// Enable Memory increment mode
	DMA1_Stream5->CR &= ~((3U << 11) | (3U << 13));	// 8-bit (1 byte) for both Memory & Peripherial data sizes
	DMA1_Stream5->NDTR = 1;									// Number of data items to transfer: 1
	DMA1_Stream5->CR |= 4 << 25;							// Channel 4 selected
}
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint8_t size)
{
	DMA1_Stream5->NDTR = size;							// the Size of the transfer
	DMA1_Stream5->PAR = srcAdd;						// Source address is Peripheral address
	DMA1_Stream5->M0AR = destAdd;						// Destination address is Memory address
	
	DMA1->HIFCR |= DMA_HIFCR_CTCIF5;					// Clear TCIFx flag by writing 1
	DMA1_Stream5->CR |= 1 << 4;						// Enable Transfer complete interrupt
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);				// Enable Stream5 interrupt in NVIC
		
	DMA1_Stream5->CR |= 1 << 0;						// Enable DMA stream
}
void DMA1_Stream5_IRQHandler(void)
{
	// Check for Stream 5 transfer complete flag
	if(DMA1->HISR & DMA_HISR_TCIF5)
	{
		Led_Blinking(buffer-0x30);
		DMA1->HIFCR |= DMA_HIFCR_CTCIF5;					// Clear TCIFx flag by writing 1
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
void Led_Blinking(int c)
{
	for(int i = 0; i < c+1; i++)
	{
		GPIOA->ODR |= (1 << 5);
		delayMs(200);
		GPIOA->ODR &= ~(1U << 5);
		delayMs(200);
	}
	buffer = 0;
}
