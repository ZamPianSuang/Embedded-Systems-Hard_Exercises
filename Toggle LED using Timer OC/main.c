#include "stm32f4xx.h"

void System_Clock_Init(void);
void LED_Init(void);
void TIM2_CH1_Init(void);

static volatile uint32_t tick;

int main(void)
{	
	System_Clock_Init();				// Switch System Clock = 80 MHz	
	LED_Init();							// Init PA5 as Alternate Function
	TIM2_CH1_Init();					// Up counting, Output-Compare mode

	while(1)
	{
		tick = (TIM2->CNT);
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
	
	RCC->CFGR &= ~(0xFU<<4);		// system clock not divided (AHB/1)
	RCC->CFGR &= ~(0x7U<<10);		// (APB1/1)
	RCC->CFGR &= ~(0x7U<<13);		// (APB2/1)

	// Enable PLL
	RCC->CR |= RCC_CR_PLLON;
	
	// Wait until PLL is ready
	while(!(RCC->CR & RCC_CR_PLLRDY));
	
	SystemCoreClockUpdate();
}
void LED_Init(void)
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
	GPIOA->OSPEEDR &= ~(3U<<10);						// Low speed
	// Set PA5 as no pull-up/pull-down
	GPIOA->PUPDR &= ~(3U<<10);						// No pull-up, No pull-down
}
void TIM2_CH1_Init(void)
{
	/* Only TIM2_CH1 is available for PA5. See Datasheet */
	// Enable Clock for TIMER2 
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;				/*  MAX 90 MHz */
	
	/* Runs APB1 Timer Clocks at 80 MHz */
	// AHB1 Prescaler: 1
	RCC->CFGR &= ~(0x0FUL<<4);						// Clear HPRE[3:0]	
	// APB1 Prescaler: 2
	RCC->CFGR &= ~(0x07UL<<10);						// Clear PPRE1[2:0]
	RCC->CFGR |= 4<<10;								// 100: AHB clock divided by 2

	// Counting direction: Up counting 
	TIM2->CR1 &= ~TIM_CR1_DIR;
	
	/*
		Fclk_cnt = Fclk_psc/(1+PSC)     = 80 MHz/40000  = 2,000 Hz
		Peroid   = (1+ARR) * 1/Fclk_cnt = 2000*(1/2000) = 1 s
		Duty 		= CCR * 1/Fclk_cnt     = 500*(1/2000)  = 0.25 s (25%)
	*/
	
	// Clock prescaler (16 bits, up to 65,535)
	TIM2->PSC = 40000-1;							// Fclock_cnt = 80MHz/8,000 = 10 KHz (10ms)
	
	// Auto-reload: up-counting(0 --> ARR), down-counting (ARR --> 0)
	TIM2->ARR = 2000-1;								// Peroid = (1+ARR)x1/Fclock_cnt = 1 second
	
	// Can be any value to compare with CNT 
	TIM2->CCR1 = 500;
	
	/* Select toggle mode (0011) */
	// Clear output compare mode bits for channel 1
	TIM2->CCMR1 &= ~TIM_CCMR1_OC1M;
	TIM2->CCMR1 |= TIM_CCMR1_OC1M_0 | TIM_CCMR1_OC1M_1;
	
	//TIM2->CR1 |= TIM_CR1_OPM;
	
	// Select output polarity: 0 = active high, 1 = active low
	TIM2->CCER &= ~TIM_CCER_CC1NP;					// Select active high
	
	// Enable output for channel 1 complementary output
	TIM2->CCER |= TIM_CCER_CC1E;
	
	// Enable Timer 2
	TIM2->CR1 |= TIM_CR1_CEN;
}
