#include "stm32f4xx.h"                  			// Device header

void GPIO_Init(void);
void TIM2_CH1_IC_Init(void);
void TIM2_IRQHandler(void);

static volatile uint32_t pulse_width = 0;
static volatile uint32_t overflow = 0;
static volatile uint32_t last_captured = 0;
static volatile uint32_t signal_polarity = 0;		// Assume input is LOW initially

int main(void)
{
	/* Using default 16 MHz HSI as system clock */
	GPIO_Init();
	TIM2_CH1_IC_Init();
	while(1)
	{

	}
}
void GPIO_Init(void)
{
	/* LED is in PA5 for STM32F446RE */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// MAX 180 MHz //
	// Set PA5 as Alternate Function
	GPIOA->MODER &= ~(3UL<<10);						// Clear bit 11 and 10
	GPIOA->MODER |= (2UL<<10);							
	// Select alternate function 0 (TIM2_CH1)
	GPIOA->AFR[0] &= ~(0xFUL<<20);					// Clear AFRL[23:20]
	GPIOA->AFR[0] |= (1<<20);						// Set AFRL5[3:0] as AF1
	// Set I/O speed value as low
	GPIOA->OSPEEDR &= ~(3U<<10);					// Low speed
	// Set PA5 as no pull-up/pull-down
	GPIOA->PUPDR &= ~(3U<<10);						// No pull-up, No pull-down
}
void TIM2_CH1_IC_Init(void)
{
	/* Only TIM2_CH1 is available for PA5. See Datasheet */
	// Enable Clock for TIMER2 
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;				/*  MAX 90 MHz */
	
	// Set up an appropriate prescaler to slow down the timer's input clock
	TIM2->PSC = 16000-1;
	
	// Set Auto-reload value to maximum value
	TIM2->ARR = 0xFFFF;					// Max 16-bit value
	
	// Set the direction of channel 1 as input, and select the active input
	TIM2->CCMR1 &= ~(3U<<0);
	TIM2->CCMR1 |= 1U;				// 01: CC1 channel is configured as input, IC1 is mapped on TI1
	
	// Program input filter duration: Disable digital filtering by clearing
	// IC1F[3:0] bits because we want to capture every event
	TIM2->CCMR1 &= ~TIM_CCMR1_IC1F;		// No filtering
	
	// Program the input prescaler: clear prescaler to capture each transition
	TIM2->CCMR1 &= ~(TIM_CCMR1_IC1PSC);
	
	// Select falling edge as the active transition to generate interrupts	
	TIM2->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC1NP);		// Cler polarity bits
	TIM2->CCER |= TIM_CCER_CC1P;									// Only falling edges generate interrupts
	
	// Select input trigger source
	TIM2->SMCR &= ~TIM_SMCR_TS;
	TIM2->SMCR |= 4U << 4;			// TI1 Edge detecctor (TI1F_ED)
	
	// Select Slave mode with reset
	TIM2->SMCR &= ~TIM_SMCR_SMS;
	TIM2->SMCR |= 4;
	
	// Allow channel 1 of Timer 2 to generate interrupts
	TIM2->DIER |= TIM_DIER_CC1IE;
	
	// Allow channel 1 of Timer 2 to generate DMA requests
	TIM2->DIER |= TIM_DIER_CC1DE;		// Optional. Required if DMA is used
	
	// Enable capture for channel 1
	TIM2->CCER |= TIM_CCER_CC1E;
	
	// Enable the timer counter
	TIM2->CR1 |= TIM_CR1_CEN;
	
	NVIC_SetPriority(TIM2_IRQn, 0);  	// Highest Urgency
	
	// Enable Timer 2 interrupt in the NVIC
	NVIC_EnableIRQ(TIM2_IRQn);
}
void TIM2_IRQHandler(void)
{
	if((TIM2->SR & TIM_SR_UIF) != 0)	// Check if overflow has taken place
		TIM2->SR &= ~TIM_SR_UIF;		// Clear UIF flag to prevent re-entering
	if((TIM2->SR & TIM_SR_CC1IF) != 0)	// Check interrupt flag is set
		pulse_width = TIM2->CCR1;		// Reading CCR1 clears CC1IF flag
}
