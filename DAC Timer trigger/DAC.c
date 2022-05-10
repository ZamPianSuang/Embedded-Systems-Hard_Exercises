#include "DAC.h"

void DAC_channel1_Init(void)
{
	// DAC channel 1: DAC_OUT1 = PA4
	
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;				// Enable GPIOA clock
	GPIOA->MODER |= GPIO_MODER_MODER4;					// PA4 analog mode
	
	RCC->APB1ENR |= RCC_APB1ENR_DACEN;					// Enable DAC clock
	DAC->CR &= ~(DAC_CR_EN1 | DAC_CR_EN2);				// Disable DAC
	DAC->CR |= DAC_CR_TEN1;									// ENABLE trigger for DAC channel 1
	DAC->CR |= (DAC_CR_TSEL1_2 | DAC_CR_TSEL1_0);	// TIM4_TRGO event trigger
	DAC->CR &= ~DAC_CR_BOFF1;								// DAC channel1 output buffer enabled
	DAC->CR |= DAC_CR_EN1;									// Enable DAC channel 1
}
void TIM4_Init(void)
{
	// 0.5 ms step size
	RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;					// Enable TIM4 clock
	TIM4->CR1 &= ~TIM_CR1_CMS;								// Clear edge-aligned mode bits
	TIM4->CR1 &= ~TIM_CR1_DIR;								// Up-counting mode
	TIM4->CR1 |= TIM_CR1_ARPE;								// TIMx_ARR register is buffered
	TIM4->CR2 |= TIM_CR2_MMS_2;							// OC1REF signal is used as trigger output (TRGO)
	// OC1M: Output Compare 1 Mode - PWM mode 1
	TIM4->CCMR1 |= (TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2);
	
	// Timer driving frequency = 16 MHz/(1+PSC) = 16 MHz/(1+18) = 842,105 Hz
	// Trigger frequency = 842,105 Hz/(1+ARR) = 842,105/(1+18) = 44.3 kHz (only 0.5% off from 44.1 kHz)
	TIM4->PSC  = 18;
	TIM4->ARR  = 18;
	TIM4->CCR1 = 9;											// Duty ratio 50%
	TIM4->CCER |= TIM_CCER_CC1E;							// OC1 signal is output
	TIM4->CR1  |= TIM_CR1_CEN;								// Enable Timer
}
