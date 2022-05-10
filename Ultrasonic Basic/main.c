// PA0 - Trigger -->> Output
// PA5 - Echo    <<-- Input
// GND - Ground
// Vcc - >=3.2V

#include "stm32f4xx.h"                  // Device header
#include "stdint.h"

void GPIO_Init(void);
void TIM2_Init(void);
int wait_sensor(void);
int get_distance(uint32_t *ticks);
void delayUs(uint32_t delay);

//static volatile uint32_t timespan = 0;
//static volatile uint32_t overflow = 0;
//static volatile uint32_t last_captured = 0;
static volatile uint32_t distance = 0;		// Assume input is LOW initially
volatile uint32_t final = 0;

int main(void)
{
	/* Using default 16 MHz HSI as system clock */
	
	GPIO_Init();
	TIM2_Init();
	
	while(1)
	{
		/* Send 10us Trigger */
		GPIOA->ODR &= ~1u;
		delayUs(5);
		GPIOA->ODR |= 1;
		delayUs(10);
		GPIOA->ODR &= ~1u;
		
		if(wait_sensor())
		{
			if(get_distance(&distance));
			else
			{
				distance = distance/58;
				final = distance;
			}
		}
		delayUs(500000);						// 0.5 s
	}
}
void GPIO_Init(void)
{
	// PA0 - Trigger -->> Output
	// PA5 - Echo    <<-- Input

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// MAX 180 MHz //
	
	GPIOA->MODER &= ~(0xC03U);					// Clear PA5 and PA0
	GPIOA->MODER |= 1 << 0;						// PA0 Output, PA5 Input
	
	GPIOA->OTYPER &= ~(0x21U);					// PA0 & PA5 as Output push-pull
	
	GPIOA->OSPEEDR |= 0xC03U;					// High speed
	
	GPIOA->PUPDR &= ~(0xC03U);					// PA5 and PA0 No Pull-up, pull-down
}
void TIM2_Init(void)
{
	/* micro-second unit */
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;				// Enable Timer 2
	// Counting direction: Up counting 
	TIM2->CR1 &= ~TIM_CR1_DIR;
	
	TIM2->SR = 0;
	TIM2->CNT = 0;
	TIM2->PSC = 160-1;								// 0 <-> 65535
	TIM2->ARR = 0xFFFF;						
	//TIM2->CR1 |= TIM_CR1_CEN;						// Enable Timer 2
	
	//while(!(TIM2->SR & TIM_SR_UIF));
}
int wait_sensor(void)
{
	uint32_t i = 0;
	TIM2->CR1 &= ~TIM_CR1_CEN;		// Timer2 off
	TIM2->CNT = 0;
	TIM2->CR1 |= TIM_CR1_CEN;		// Timer2 on
	i = TIM2->CNT;
	while(!(GPIOA->IDR & (1U << 5)) && i < 1000);
		
	if(i >= 1000) return 0;
	else
		return 1;
}
int get_distance(uint32_t *ticks)
{
	*ticks = 0;
	TIM2->CNT = 0;
	while((GPIOA->IDR & (1U << 5)) && (*ticks < 22857))
		*ticks = TIM2->CNT;
	TIM2->CR1 &= ~TIM_CR1_CEN;		// Timer2 off
	if(*ticks >= 22857) return 1;
	else
		return 0;
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
