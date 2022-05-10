#include "stm32f4xx.h"

void System_Clock_Init(void);
void systickDelayMs(int delay);
void LED_Init(void);

int main(void)
{	
	System_Clock_Init();				// Switch System Clock = 80 MHz	
	LED_Init();									// Init PA5 as Alternate Function

	while(1)
	{
		GPIOA->ODR ^= 1<<5;
		systickDelayMs(1000);
	}
}

void System_Clock_Init(void)
{
	/* SYSCLK - 180 MHz */ // HSE = 20MHz //
	#define PLL_M	10
	#define PLL_N	180
	#define PLL_P	0 			// PLL_P = 2
	
	// Enable HSE
	RCC->CR |= RCC_CR_HSEON;
	
	// Wait until HSI is ready
	while(!(RCC->CR & RCC_CR_HSERDY));
	
	// 2. Set POWER ENABLE CLOCK and VOLTAGE REGULATOR
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_VOS;
	
	// Activate prefetch buffer but it should already be on
	FLASH->ACR |= FLASH_ACR_PRFTEN;
	
	// 7 Wait State (168 <HCLK <= 180) ** Very Important Part **
	FLASH->ACR &= ~(FLASH_ACR_LATENCY);
	FLASH->ACR |= 0x7U;
	
	/* It is recommended to configure PLL before enabling it */
	RCC->CR &= ~RCC_CR_PLLON;
	
	// Select HSE as PLL entry clock source
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSE;
	
	// VCO input frequency = PLL input clock frequency / PLLM[5:0] (VCO = 8/8)
	RCC->PLLCFGR &= ~(0x3FU);		// clear RCC_PLLCFGR [5:0]
	RCC->PLLCFGR |= PLL_M << 0;				// VCO input = 16/8 = 2 MHz
	// VCO output frequency = VCO input frequency × PLLN[8:0]
	RCC->PLLCFGR &= ~(0x7FC0U);		// Clear RCC_PLLCFGR [14:6]
	RCC->PLLCFGR |= PLL_N << 6;			// VCO output = 2*80 = 160 MHz
	// PLL output clock frequency = VCO frequency / PLLP[1:0]
	RCC->PLLCFGR |= PLL_P << 16; 		// PLLCLK output = 160/2 = 80 MHz	
	
	// Select PLLCLK as the clock source of System Clock (SYSCLK)
	RCC->CFGR |= RCC_CFGR_SW_PLL;
	//while((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);
	
	RCC->CFGR &= ~(0xFU<<4);		// system clock not divided (AHB/1)
	RCC->CFGR &= ~(0x7U<<10);		
	RCC->CFGR |= 5 << 10;			// (APB1/4)
	RCC->CFGR &= ~(0x7U<<13);		
	RCC->CFGR |= 4 << 13;			// (APB2/2)
	
	// Enable PLL
	RCC->CR |= RCC_CR_PLLON;
	
	// Wait until PLL is ready
	while(!(RCC->CR & RCC_CR_PLLRDY));
	
	SystemCoreClockUpdate();
}

void systickDelayMs(int delay)	// see User Guide Note: LOAD is automatically reload again n again
{
	// Any value between 0x00000001 - 0x00FFFFFF (2^24 - 1) 16,777,215
	SysTick->LOAD = 180000-1; 			// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;					// Clear Current Value Register
	SysTick->CTRL &= ~(0x10007U);		// Reset all
	SysTick->CTRL |= 0x05;				// 101 - 1:processor clock, 1:counter enabled
										// 0:counting down to zero does not assert the SysTick exception request
	
	for (int i = 0; i < delay; i++)		//[16]Returns 1 if timer counted to 0 since last time this was read. 
	{
		// Wait until the COUNT flag is set
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	
	SysTick->CTRL = 0;					// stop the timer
}

void LED_Init(void)
{
	// LED is in PA5 for STM32F446RE //
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// MAX 180 MHz //
	// Set PA5 as Output Function
	GPIOA->MODER &= ~(3U<<10);								// Clear bit 11 and 10
	GPIOA->MODER |= (1U<<10);		
	// PA5: Output push-pull
	GPIOA->OTYPER &= ~(1U<<5);
	// PA5: High speed
	GPIOA->OSPEEDR &= ~(3U<<10);								// Clear bit 11 and 10
	GPIOA->OSPEEDR |= (3U<<10);
	// PA5: No pull-up/pull-down
	GPIOA->PUPDR &= ~(3U<<10);
}
