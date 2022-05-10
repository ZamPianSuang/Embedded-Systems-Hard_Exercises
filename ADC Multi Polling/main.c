#include "stm32f4xx.h"                  // Device header

volatile uint32_t ADC_VAL[2] = {0,0};

void ADC_Init(void);
void ADC_Enable(void);
void ADC_Start(unsigned int channel);
void ADC_Enable(void);
void ADC_Disable(void);
uint32_t ADC_GetVal(void);

int main(void)
{
	ADC_Init();
	ADC_Enable();
	
	while(1)
	{
		ADC_Start(1);
		ADC_VAL[0] = ADC_GetVal();
		
		ADC_Start(4);
		ADC_VAL[1] = ADC_GetVal();
	}
}
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
	
	// 4. Set the Scan mode and Resolution in CR1
	ADC1->CR1 &= ~ADC_CR1_RES;					// 12 bit resolution
	ADC1->CR1 |= ADC_CR1_SCAN;					// Scan mode enable
	
	// 5. Set the Continuous conversion, EOC, and Data Alignment in CR2
	ADC1->CR2 |= ADC_CR2_CONT;					// Continuous conversion mode 
	ADC1->CR2 |= ADC_CR2_EOCS;					// EOC after each conversion
	ADC1->CR2 &= ~ADC_CR2_ALIGN;				// Right alignment
	
	// 6. Set the Sampling time for the channels
	ADC1->SMPR2 &= ~(ADC_SMPR2_SMP1 | ADC_SMPR2_SMP4);
	
	// 7. Set the Regular channel sequence Length
	ADC1->SQR1 &= ADC_SQR1_L;
	ADC1->SQR1 |= ADC_SQR1_L_0;				// 2 conversions
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
void ADC_Start(unsigned int channel)
{
	ADC1->SQR3 = 0;
	ADC1->SQR3 |= (channel << 0);
	ADC1->SR = 0;
	ADC1->CR2 |= ADC_CR2_SWSTART;
}
uint32_t ADC_GetVal(void)
{
	while(!(ADC1->SR & ADC_SR_EOC));			// wait for EOC flag to set
	return ADC1->DR;
}
