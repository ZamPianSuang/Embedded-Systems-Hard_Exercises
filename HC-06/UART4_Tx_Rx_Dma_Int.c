#include "UART4_Tx_Rx_Dma_Int.h"

volatile unsigned char buffer;

void PC10_PC11_UART4_Init(void)
{
	// Initialize PC10 and PC11 as Alternate function mode
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER |= GPIO_MODER_MODE5_0;						// PA5 for blinking function
	
	GPIOC->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11);
	GPIOC->MODER |= GPIO_MODER_MODE10_1 | GPIO_MODER_MODE11_1;		// Alternate function mode
	GPIOC->OTYPER &= ~(GPIO_OTYPER_OT10 | GPIO_OTYPER_OT11);			// Output push-pull
	GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11);		// No pull-up/ pull-down
	GPIOC->AFR[1] |= GPIO_AFRH_AFRH2_3 | GPIO_AFRH_AFRH3_3;			// PC10 & PC11 as AF8
	
	UART4_Tx_Rx_DMA_Init();
}
void UART4_Tx_Rx_DMA_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_UART4EN;					// 45 MHz max
	
	UART4->CR1 &= ~USART_CR1_UE;								// Disable UART4
	
	UART4->CR1 &= ~USART_CR1_M;								// 1 Start bit, 8 Data bits, n Stop bit
	UART4->CR2 &= ~USART_CR2_STOP;							// 1 Stop bit
	UART4->CR1 &= ~USART_CR1_OVER8;							// Oversampling by 16
	UART4->BRR = 0x683;											// 9600 baut rate @16 MHz
	UART4->CR1 |= USART_CR1_TE;								// Enable Transmission
	UART4->CR1 |= USART_CR1_RE;								// Enable Reception 
	UART4->CR1 |= USART_CR1_UE;								// Enable UART4
	
	/****************************** DMA for Rx ******************************/
	UART4->CR3 |= USART_CR3_DMAR;								// Enable DMA receiver
}
void DMA_Init(void)
{
	/* USART2_Rx - DMA1, Channel4, Stream5 on F446RE */
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;						// Enable clock for DMA1
	
	DMA1_Stream2->CR &= ~(3U << 6);							// Peripherial to Memory
	DMA1_Stream2->CR |= 1 << 8;								// Circular mode
	DMA1_Stream2->CR &= ~(1U << 9);							// Disable peripherial increment mode
	DMA1_Stream2->CR |= 1 << 10;								// Enable Memory increment mode
	DMA1_Stream2->CR &= ~((3U << 11) | (3U << 13));		// 8-bit (1 byte) for both Memory & Peripherial data sizes
	DMA1_Stream2->NDTR = 1;										// Number of data items to transfer: 1
	DMA1_Stream2->CR |= 4 << 25;								// Channel 4 selected
}
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint8_t size)
{
	DMA1_Stream2->NDTR = size;									// the Size of the transfer
	DMA1_Stream2->PAR = srcAdd;								// Source address is Peripheral address
	DMA1_Stream2->M0AR = destAdd;								// Destination address is Memory address
	
	DMA1->LIFCR |= DMA_LIFCR_CTCIF2;							// Clear TCIFx flag by writing 1
	DMA1_Stream2->CR |= 1 << 4;								// Enable Transfer complete interrupt
	
	NVIC_EnableIRQ(DMA1_Stream2_IRQn);						// Enable Stream5 interrupt in NVIC
		
	DMA1_Stream2->CR |= 1 << 0;								// Enable DMA stream
}
void DMA1_Stream2_IRQHandler(void)
{
	// Check for Stream 2 transfer complete flag
	if(DMA1->LISR & DMA_LISR_TCIF2)
	{
		Led_Blinking(buffer-0x30);
		DMA1->LIFCR |= DMA_LIFCR_CTCIF2;					// Clear TCIFx flag by writing 1
	}
}
void delayMs(int delay)	
{
	SysTick->LOAD = 16000-1; 	
	SysTick->VAL = 0;					
	SysTick->CTRL |= 0x05;		
									
	for (int i = 0; i < delay; i++)		
	{
		while((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;			
}
void Led_Blinking(int c)
{
	for(int i = 0; i < c+1; i++)
	{
		GPIOA->ODR &= ~(1U << 5);
		delayMs(200);
		GPIOA->ODR |= (1 << 5);
		delayMs(200);
	}
	buffer = 0;
}
