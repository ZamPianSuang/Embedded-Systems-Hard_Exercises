#include "stm32f4xx.h"                  // Device header

void LED_Init(void);
void TIM2_CH1_Init(void);

static volatile uint32_t brightness = 1;
static volatile uint32_t stepSize = 1;

int main(void)
{
	/* Using default HSI 16 MHz as System Clock */
	
	int i;
	
	LED_Init();
	TIM2_CH1_Init();
	while(1)
	{
		if((brightness >= 999) || (brightness <= 0))
			stepSize = -stepSize;
		brightness += stepSize;
		TIM2->CCR1 = brightness;
		for(i = 0; i < 1600; i++);
	}
}
void LED_Init(void)
{
	/* PA5 on STM32F446re */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	
	GPIOA->MODER |= 2U << 10;				// Alternate Function Mode
	GPIOA->OTYPER &= ~(1U<<5);				// Output Pull-pull  
	GPIOA->OSPEEDR &= ~(3U << 10);			// Low speed
	GPIOA->PUPDR &= ~(3U << 10);			// No pull-up, pull-down
	
	GPIOA->AFR[0] &= ~(0xFU << 20);			// Clear AFRL5[3:0]
	GPIOA->AFR[0] |= 1U << 20;				// Activate AF1
}
void TIM2_CH1_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;		
	
	TIM2->CR1 &= ~TIM_CR1_DIR;				// Upcounting
	// (1+PSC) = Timer/Counter -- Counter -> 100KHz
	TIM2->PSC = 160-1;						// (1+PSC) = 16 MHz/100 KHz = 160
	// (1+ARR) = Counter/PWM -- (1/100KHz)*1000 = 0.01s
	TIM2->ARR = 1000-1;						// (1+ARR) = 100 KHz/100% = 1000
	TIM2->CCMR1 |= 6 << 4;					// PWM Mode 1 on channel 1
	TIM2->CCMR1 |= TIM_CCMR1_OC1PE;			// Output 1 preload enable
	TIM2->CCER &= ~TIM_CCER_CC1NP;			// OC1N = OCREF + CC1NP
	// Enable output for channel 1 complementary output
	TIM2->CCER |= TIM_CCER_CC1E;

	TIM2->CCR1 = 0;
	TIM2->CR1 |= TIM_CR1_CEN;
}
