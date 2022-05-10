// PA0 - Trigger -->> TIM5_CH1
// PA5 - Echo    <<-- TIM2_CH1
// GND - Ground
// Vcc - >= 3.2V

#include "stm32f4xx.h"                  // Device header

void GPIO_Init(void);
void TIM5_CH1_Init(void);
void TIM2_CH1_IC_Init(void);
void TIM2_IRQHandler(void);
void start_trigger(void);
void delayUs(uint32_t delay);

static volatile uint32_t pulse_width = 0;
static volatile uint32_t timespan = 0;
static volatile uint32_t overflow = 0;
static volatile uint32_t last_captured = 0;
static volatile uint32_t distance = 0;
static volatile uint32_t signal_polarity = 0;		// Assume input is LOW initially

int main(void)
{
	/* Using default 16 MHz HSI as system clock */
	
	GPIO_Init();
	TIM5_CH1_Init();
	TIM2_CH1_IC_Init();
	
	while(1)
	{
		start_trigger();
		
		
		delayUs(500000);			// half second
		
	}
}
void GPIO_Init(void)
{
	// PA0 - Trigger -->> TIM5_CH1
	// PA5 - Echo    <<-- TIM2_CH1

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// MAX 180 MHz //
	// Set as Alternate Function
	GPIOA->MODER &= ~(3UL << 0);
	GPIOA->MODER &= ~(3UL << 10);					
	GPIOA->MODER |= (2UL << 0);						// PA0 - Alternate
	GPIOA->MODER |= (2UL << 10);						// PA5 - Alternate 
	
	// PA0 - TIM5_CH1 - AF2, PA5 - TIM2_CH1 - AF1
	GPIOA->AFR[0] &= ~(0xFU << 0);
	GPIOA->AFR[0] |= 2U << 0;							// PA0 - AF2
	GPIOA->AFR[0] &= ~(0xFU << 20);				
	GPIOA->AFR[0] |= (1 << 20);						// PA5 - AF1
	
	// Set I/O speed value 
	GPIOA->OSPEEDR |= (3U << 0);						// PA0 - High speed
	GPIOA->OSPEEDR |= (3U << 10);						// PA5 - High speed
	
	// Set PA5 as no pull-up/pull-down
	GPIOA->PUPDR &= ~(3U << 0);						// No pull-up, No pull-down
	GPIOA->PUPDR &= ~(3U << 10);						// No pull-up, No pull-down
}
void TIM5_CH1_Init(void)
{
	// PA0 - Trigger -->> TIM5_CH1
	
	/* Initialization as PWM Output Mode */
	RCC->APB1ENR |= RCC_APB1ENR_TIM5EN;
	
	TIM5->CR1   &= ~TIM_CR1_DIR;					// Upcounting mode
	
	/** Counter Clock = Input Clock/1+PSC = 16 MHz/1+15 = 1 MHz **/
	/** PWM period = (1+ARR) * Counter Clock Period = (1+0xFFFF) * (1/1MHz) = 0.065536 s **/
	/** PWM pulse width = CCR * Counter clock period = 10 * (1/1MHz) = 10 us or 0.00001 s**/
	
	TIM5->PSC |= 16-1;								// freq => 16 MHz / (240*65535) ~= 1 Hz (1 s)
	TIM5->ARR = 0xFFFF;								// duty => 1 / 65536 ~= 15.5 us
	TIM5->CCR1 = 10;
	
	TIM5->CCER  &= ~TIM_CCER_CC1P;				// Active high
	TIM5->CCER  &= ~TIM_CCER_CC1NP;
	TIM5->CCMR1 |= 6 << 4;							// PWM Output Mode 1
	TIM5->CCMR1 |= TIM_CCMR1_OC1PE;				// Enable preload register
	TIM5->CR1   |= TIM_CR1_ARPE;					// 1: TIMx_ARR register is buffered
	TIM5->EGR   |= TIM_EGR_UG;						// Update generation
	TIM5->CCMR1 &= ~3U << 0;

	TIM5->CCER  |= TIM_CCER_CC1E;					// Enable Capture/Compare 1 output 
	TIM5->CR1 |= TIM_CR1_CEN;						// Enable Timer 5
}
void start_trigger(void)
{
	__disable_irq();
	TIM5->CNT = 0;
	while(TIM5->CNT <= TIM5->ARR);				// Wait 1 period
	__enable_irq();
}

void TIM2_CH1_IC_Init(void)
{
	/* Only TIM2_CH1 is available for PA5. See Datasheet */
	__disable_irq();
	// Enable Clock for TIMER2 
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;				/*  MAX 90 MHz */
	
	// Set up an appropriate prescaler to slow down the timer's input clock
	TIM2->PSC = 16-1;							// timespan in Ms rate
	TIM2->ARR = 0xFFFF;					// Max 16-bit value
	
	// Set the direction of channel 1 as input, and select the active input
	TIM2->CCMR1 &= ~(3U<<0);
	TIM2->CCMR1 |= 1U;				// 01: CC1 channel is configured as input, IC1 is mapped on TI1
	
	// Program input filter duration: Disable digital filtering by clearing
	// IC1F[3:0] bits because we want to capture every event
	TIM2->CCMR1 &= ~TIM_CCMR1_IC1F;		// No filtering
	
	// Set the active transition as both rising and falling edges
	// CC1NP:CC1P bits: 11 = both edges
	TIM2->CCER |= TIM_CCER_CC1P | TIM_CCER_CC1NP;
	
	// Program the input prescaler: clear prescaler to capture each transition
	//TIM2->CCMR1 &= ~(TIM_CCMR1_IC1PSC);
	
	TIM2->DIER |= TIM_DIER_UIE;
	
	// Allow channel 1 of Timer 2 to generate interrupts
	TIM2->DIER |= TIM_DIER_CC1IE;
	
	// Allow channel 1 of Timer 2 to generate DMA requests
	TIM2->DIER |= TIM_DIER_CC1DE;		// Optional. Required if DMA is used
	
	// Enable capture for channel 1
	TIM2->CCER |= TIM_CCER_CC1E;
	
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
			pulse_width = current_captured - last_captured;		// Assume up-counting
		
			timespan = pulse_width + (65536 * overflow);
		
			distance = timespan/58;
		
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

void delayUs(uint32_t delay)
{
	// Any value between 0x00000001 - 0x00FFFFFF (2^24 - 1)
	SysTick->LOAD = 16-1; 						// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL  = 0;								// Clear Current Value Register
	SysTick->CTRL &= ~(0x10007U);					// Reset all
	SysTick->CTRL |= 0x05;							// 101 - 1:processor clock, 1:counter enabled
															// 0:counting down to zero does not assert the SysTick exception request
	for (uint32_t i = 0; i < delay; i++)		//[16]Returns 1 if timer counted to 0 since last time this was read. 
	{
		// Wait until the COUNT flag is set
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;								// stop the timer
}
