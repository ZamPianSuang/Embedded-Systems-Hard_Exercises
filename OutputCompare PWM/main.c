#include "stm32f4xx.h"                  	// Device header

void LED_Init(void);
void TIM2_CH1_PWM_Init(void);
void PWM_DutyCycle(uint16_t c);				// by percentage (0-100)

static volatile uint16_t tick = 0;

int main(void)
{
	/* Using default 16 MHz HSI as SYSCLK */
	LED_Init();
	TIM2_CH1_PWM_Init();
	
	while(1)
	{
		tick = TIM2->CNT;
		PWM_DutyCycle(1);
	}
}
void LED_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER |= 2U << 10;				// Alternate function mode
	GPIOA->OTYPER &= ~(1U << 5);			// Output push-pull
	GPIOA->OSPEEDR &= ~(3U << 10);			// Low speed
	GPIOA->PUPDR &= ~(3U << 10);			// No pull-up, pull-down
	GPIOA->AFR[0] &= ~(0xFU << 20);
	GPIOA->AFR[0] |= 1U << 20;				// AF1
}
void TIM2_CH1_PWM_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
	
	TIM2->CR1 &= ~TIM_CR1_DIR;				// Upcounting mode
	TIM2->CCMR1 |= 6 << 4;					// PWM mode 1 for CH1
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;			// Output compare 1 preload enable
	TIM2->CR1 |= TIM_CR1_ARPE;				// TIMx_ARR register is buffered
	TIM2->CCER &= ~TIM_CCER_CC1P; 			// 0: OC1 active high
	TIM2->CCER |= TIM_CCER_CC1E;			// 1: OC1 signal is output on the corresponding output pin
	
	/* Counter Clock = Input Clock/1+PSC = 16 MHz/1+159 = 0.1MHz */
	/* PWM period = ARR * Counter Clock Period = 1000 * (1/0.1MHz) = 0.01s or 10 ms */
	/* PWM pulse width = CCR * Counter clock period = (20%) 200 * (1/0.1MHz) = 0.002s or 2 ms */
  
	TIM2->PSC = 160-1;
	TIM2->ARR = 1000;
	TIM2->CCR1 = 0;		// 200/1000 (20%) = 0.2
	
	// Enable output for channel 1 complementary output
	//TIM2->CCER |= TIM_CCER_CC1E;
	
	TIM2->EGR |= TIM_EGR_UG;				// Enable Update Generation	
	TIM2->CR1 |= TIM_CR1_CEN;				// Enable Timer 2
	
}
void PWM_DutyCycle(uint16_t c)
{
	if( c > 100) c = 100;
	TIM2->CCR1 = c * 10;
}

