#include "stm32f4xx.h"                  // Device header

void System_Clock_Init(void);
void LED_Init(void);
void TIM2_Init(void);
void TIM2_IRQHandler(void);

static volatile uint32_t overflow = 0;

int main(void)
{
	__disable_irq();
	System_Clock_Init();
	LED_Init();
	TIM2_Init();
	__enable_irq();
	
	while(1)
	{
		
	}
}
void System_Clock_Init(void)
{
	/* SYSCLK - 80 MHz */
	// Enable HSI
	RCC->CR |= RCC_CR_HSION;
	
	// Wait until HSI is ready
	while(!(RCC->CR & RCC_CR_HSIRDY));
	
	// Activate prefetch buffer but it should already be on
	FLASH->ACR |= FLASH_ACR_PRFTEN;
	
	// 3 Wait State (72 < HCLK = 96) ** Very Important Part **
	FLASH->ACR &= ~(FLASH_ACR_LATENCY);
	FLASH->ACR |= 0x3U;
	
	/* It is recommended to configure PLL before enabling it */
	RCC->CR &= ~RCC_CR_PLLON;
	
	// Select HSI as PLL entry clock source
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;
	
	// VCO input frequency = PLL input clock frequency / PLLM[5:0] (VCO = 8/8)
	RCC->PLLCFGR &= ~(0x3FU);		// clear RCC_PLLCFGR [5:0]
	RCC->PLLCFGR |= 8;				// VCO input = 16/8 = 2 MHz
	// VCO output frequency = VCO input frequency × PLLN[8:0]
	RCC->PLLCFGR &= ~(0x7FC0U);		// Clear RCC_PLLCFGR [14:6]
	RCC->PLLCFGR |= 80<<6;			// VCO output = 2*80 = 160 MHz
	// PLL output clock frequency = VCO frequency / PLLP[1:0]
	RCC->PLLCFGR &= ~(3U<<16); 		// PLLCLK output = 160/2 = 80 MHz	
	
	// Select PLLCLK as the clock source of System Clock (SYSCLK)
	RCC->CFGR |= RCC_CFGR_SW_1;
	/*
	RCC->CFGR &= ~(0xFU<<4);		// system clock not divided (AHB/1)
	RCC->CFGR &= ~(0x7U<<10);		// (APB1/1)
	RCC->CFGR &= ~(0x7U<<13);		// (APB2/1)
	*/
	// Enable PLL
	RCC->CR |= RCC_CR_PLLON;
	
	// Wait until PLL is ready
	while(!(RCC->CR & RCC_CR_PLLRDY));
	
	SystemCoreClockUpdate();
}
void LED_Init(void)
{
	// LED is in PA5 for STM32F446RE //
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// MAX 180 MHz //
	// Set PA5 as Output Function
	GPIOA->MODER &= ~(3U<<10);						// Clear bit 11 and 10
	GPIOA->MODER |= (1U<<10);		
	// PA5: Output push-pull
	GPIOA->OTYPER &= ~(1U<<5);
	// PA5: Fast speed
	GPIOA->OSPEEDR &= ~(3U<<10);					// Clear bit 11 and 10
	GPIOA->OSPEEDR |= (1U<<11);
	// PA5: No pull-up/pull-down
	GPIOA->PUPDR &= ~(3U<<10);
}
void TIM2_Init(void)
{	
	/* Runs APB1 Timer Clocks at 80 MHz */
	// AHB1 Prescaler: 1
	RCC->CFGR &= ~(0x0FUL<<4);						// Clear HPRE[3:0]	
	// APB1 Prescaler: 2
	RCC->CFGR &= ~(0x07UL<<10);						// Clear PPRE1[2:0]
	RCC->CFGR |= 4<<10;								// 100: AHB clock divided by 2
	
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;				// Enable Timer 2
	
	// Counting direction: Up counting 
	TIM2->CR1 &= ~TIM_CR1_DIR;
	
	// 1 second interrupt
	TIM2->PSC = 8000-1;								//80MHz/8,000 = 10KHz(10ms)
	TIM2->ARR = 10000-1;								//10KHz/10,000 = 1Hz (1s)
	
	// Enable update interrupts
	TIM2->DIER |= TIM_DIER_UIE;
	
	// Enable TIM2 interrupt in NVIC
	NVIC_EnableIRQ(TIM2_IRQn);
	
	// Enable the counter
	TIM2->CR1 |= TIM_CR1_CEN;
}	
void TIM2_IRQHandler(void)
{
	// Check whether an overflow event has taken place
	if((TIM2->SR & TIM_SR_UIF) != 0)
	{
		// increment overflow counter
		overflow++;
		
		GPIOA->ODR ^= 1<<5;
		
		// Clear flag UIF to prevent reentrance
		TIM2->SR &= ~TIM_SR_UIF;
	}
}
