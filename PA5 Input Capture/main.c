#include "stm32f4xx.h"                  // Device header

void GPIO_Init(void);
void TIM2_CH1_IC_Init(void);
void TIM2_IRQHandler(void);

static volatile uint32_t pulse_width = 0;
static volatile uint32_t overflow = 0;
static volatile uint32_t timespanMs = 0;
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
	__disable_irq();
	/* Only TIM2_CH1 is available for PA5. See Datasheet */
	// Enable Clock for TIMER2 
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;				/*  MAX 90 MHz */
	
	// Set up an appropriate prescaler to slow down the timer's input clock
	TIM2->PSC = 16000-1;			// timespan in Ms rate
	
	// Set Auto-reload value to maximum value
	TIM2->ARR = 0xFFFF;					// Max 16-bit value
	
	// Set the direction of channel 1 as input, and select the active input
	TIM2->CCMR1 &= ~(3U<<0);
	TIM2->CCMR1 |= 1U;				// 01: CC1 channel is configured as input, IC1 is mapped on TI1
	
	// Program input filter duration: Disable digital filtering by clearing
	// IC1F[3:0] bits because we want to capture every event
	TIM2->CCMR1 &= ~TIM_CCMR1_IC1F;		// No filtering
	TIM2->CCMR1 |= 2U << 4;					// 0010: fSAMPLING=fCK_INT, N=4
	
	// Set the active transition as both rising and falling edges
	// CC1NP:CC1P bits: 11 = both edges
	TIM2->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP;
	
	// Enable capture for channel 1
	TIM2->CCER |= TIM_CCER_CC1E;
	
	// Program the input prescaler: clear prescaler to capture each transition
	//TIM2->CCMR1 &= ~(TIM_CCMR1_IC1PSC);
	
	// Allow channel 1 of Timer 2 to generate interrupts
	TIM2->DIER |= TIM_DIER_CC1IE;
	
	// Allow channel 1 of Timer 2 to generate DMA requests
	TIM2->DIER |= TIM_DIER_CC1DE;		// Optional. Required if DMA is used
	
	// Enable the timer counter
	TIM2->CR1 |= TIM_CR1_CEN;
	
	NVIC_SetPriority(TIM2_IRQn, 0);  	// Highest Urgency
	
	__enable_irq();
	
	// Enable Timer 2 interrupt in the NVIC
	NVIC_EnableIRQ(TIM2_IRQn);
	
	
}
void TIM2_IRQHandler(void)
{
	uint32_t current_captured;
	
	if((TIM2->SR & TIM_SR_CC1IF) != 0)
	{
		// Reading CCR1 clears CC1IF flag
		current_captured = TIM2->CCR1;
		
		// Toggle the polarity flag
		signal_polarity = 1 - signal_polarity;
		
		if(signal_polarity == 0)
		{
			timespanMs = 0;
			pulse_width = current_captured - last_captured;		// Assume up-counting
			timespanMs = pulse_width + (65536 * overflow);
			overflow = 0;
		}
			
		last_captured = current_captured;
	}
	if((TIM2->SR & TIM_SR_UIF) != 0)	// Check if overflow has taken place
	{
		overflow++;
		TIM2->SR &= ~TIM_SR_UIF;		// Clear UIF flag to prevent re-entering
	}
}
