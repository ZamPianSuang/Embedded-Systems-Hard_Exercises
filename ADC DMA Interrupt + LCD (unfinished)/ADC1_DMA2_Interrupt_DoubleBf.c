#include "ADC1_DMA2_Interrupt_DoubleBf.h"

void ADC_Init(void)
{
	// 1. Enable ADC and GPIO clock
	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	// 2. Set the Respective GPIO pin in analog mode
	GPIOA->MODER |= 3 << 2;						// analog mode for PA1
	GPIOA->MODER |= 3 << 8;						// analog mode for PA4
	
	// 3. Set the prescalar in the Common Control Register (CCR)
	ADC->CCR &= ~ADC_CCR_ADCPRE;				// Not divided
	ADC->CCR |= ADC_CCR_ADCPRE_0;				// PCLK2 divided by 4 (80/4 = 20)
	
	// 4. Set the Scan mode and Resolution in CR1
	ADC1->CR1 &= ~ADC_CR1_RES;					// 12 bit resolution
	ADC1->CR1 |= ADC_CR1_SCAN;					// Scan mode enable
	
	// 5. Set the Continuous conversion, EOC, and Data Alignment in CR2
	ADC1->CR2 |= ADC_CR2_CONT;					// Continuous conversion mode 
	ADC1->CR2 |= ADC_CR2_EOCS;					// EOC after each conversion
	ADC1->CR2 &= ~ADC_CR2_ALIGN;				// Right alignment
	
	// 6. Set the Sampling time for the channels
	ADC1->SMPR2 &= ~(ADC_SMPR2_SMP1 | ADC_SMPR2_SMP4);		// 3 cycles sampling time
	
	// 7. Set the Regular channel sequence Length
	ADC1->SQR1 &= ADC_SQR1_L;
	ADC1->SQR1 |= ADC_SQR1_L_0;				// 2 conversions
	
	/*********************************** DMA ***********************************/
	// Enable DMA and Continuous Request
	ADC1->CR2 |= ADC_CR2_DDS | ADC_CR2_DMA;
	
	// Configure channel sequences
	ADC1->SQR3 = 0;
	ADC1->SQR3 |= ADC_SQR3_SQ1_0;				// SEQ1 for channel 1
	ADC1->SQR3 |= ADC_SQR3_SQ2_2;				// SEQ2 for channel 4
}
void ADC_Enable(void)
{
	ADC1->CR2 |= ADC_CR2_ADON;
	uint32_t delay = 10000;
	while(delay--);
}
void ADC_Disable(void)
{
	ADC1->CR2 &= ~ADC_CR2_ADON;
}
void ADC_Start(void)
{
	ADC1->SR = 0;
	ADC1->CR2 |= ADC_CR2_SWSTART;
}
uint32_t ADC_GetVal(void)
{
	while(!(ADC1->SR & ADC_SR_EOC));			// wait for EOC flag to set
	return ADC1->DR;
}
void DMA_Init(void)
{
	/* ADC1 is in Channel 0, Stream 0 or 4 of DMA2 on F446RE */
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA2EN;				// ENABLE clock for DMA2
	
	DMA2_Stream0->CR &= ~(3U << 6);					// Peripherial to Memory
	DMA2_Stream0->CR |= 1 << 8;						// Circular mode
	DMA2_Stream0->CR &= ~(1U << 9);					// Disable peripherial increment mode
	DMA2_Stream0->CR |= 1 << 10;						// Enable Memory increment mode
	DMA2_Stream0->CR |= (1 << 11) | (1 << 13);	// 16-bit for both Memory & Peripherial data sizes
	DMA2_Stream0->NDTR = 2;								// Length of ADC sequence = 2
	DMA2_Stream0->CR &= ~(7U << 25);					// Channel 0 selected
	
	/*********************************** Interrupt ***********************************/
	DMA2_Stream0->CR |= DMA_SxCR_TCIE;				// Enable Transfer complete interrupt
	NVIC_EnableIRQ(DMA2_Stream0_IRQn);				// Enable DMA interrupt
}
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint16_t size)
{
	DMA2_Stream0->NDTR = size;							// the Size of the transfer
	DMA2_Stream0->PAR = srcAdd;						// Source address is Peripheral address
	DMA2_Stream0->M0AR = destAdd;						// Destination address is Memory address
		
	DMA2_Stream0->CR |= 1 << 0;						// Enable DMA stream
}
void DMA2_Stream0_IRQHandler(void)
{
	if(!(DMA2->LISR & DMA_LISR_TCIF0))
	{
		uint32_t delay = 4000000;
		while(delay--);
		GPIOA->ODR |= 1 << 5;
		DMA2->LIFCR |= DMA_LIFCR_CTCIF0;				// Write 1 to clear TCIF flag
	}
}
void LED_Init(void)
{
	// LED is in PA5 for STM32F446RE //
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// MAX 180 MHz //
	// Set PA5 as Output Function
	GPIOA->MODER &= ~(3U<<10);					// Clear bit 11 and 10
	GPIOA->MODER |= (1U<<10);		
	// PA5: Output push-pull
	GPIOA->OTYPER &= ~(1U<<5);
	// PA5: Fast speed
	GPIOA->OSPEEDR &= ~(3U<<10);				// Clear bit 11 and 10
	GPIOA->OSPEEDR |= (1U<<11);
	// PA5: No pull-up/pull-down
	GPIOA->PUPDR &= ~(3U<<10);
}
