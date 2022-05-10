#include "RTC.h"

void LED_Init(void)
{
	// LED is in PA5 for STM32F446RE //
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;			// MAX 180 MHz //
	// Set PA5 as Output Function
	GPIOA->MODER &= ~(3U<<10);							// Clear bit 11 and 10
	GPIOA->MODER |= (1U<<10);		
	// PA5: Output push-pull
	GPIOA->OTYPER &= ~(1U<<5);
	// PA5: Fast speed
	GPIOA->OSPEEDR &= ~(3U<<10);						// Clear bit 11 and 10
	GPIOA->OSPEEDR |= (1U<<11);
	// PA5: No pull-up/pull-down
	GPIOA->PUPDR &= ~(3U<<10);
	/* Reset PA5 Initially */
	GPIOA->ODR &= ~(1U << 5);		
}

void RTC_Init(void)
{
	// RTC clock: (1) LSE is in backup domain (2) HSE and LSI are not
	RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Enable Power interface clock 
	
	// Enable access to RTC and RTC Backup registers and backup SRAM 
	PWR->CR |= PWR_CR_DBP;
	// Wait until backup domain write protection has been disabled
	while(!(PWR->CR & PWR_CR_DBP));
	
	// Reset LSEON and LSEBYP bits before configuring LSE
	RCC->BDCR &= ~(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);
	
	// RTC clock selection can be changed only if the backup domain is reset
	RCC->BDCR |= RCC_BDCR_BDRST;
	RCC->BDCR &= ~RCC_BDCR_BDRST;
	
	// Enable LSE and wait for ready
	RCC->BDCR |= RCC_BDCR_LSEON;
	while(!(RCC->BDCR & RCC_BDCR_LSERDY));
	
	// Select LSE as the RTC clock
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;
	
	// Enable the clock of RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;
}

void RTC_WKUP_IRQHandler(void)
{
	if((RTC->ISR & RTC_ISR_WUTF))
	{
		GPIOA->ODR |= (1U << 5);

		RTC->ISR &= ~RTC_ISR_WUTF;		
		
	}
	// Clear the EXTI line 22
	EXTI->PR = EXTI_PR_PR22;
}
void RTC_Wakeup_Configuration(void)
{
	// Disable Write protection
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	RTC->CR &= ~RTC_CR_WUTIE;			// Disable Wakeup Interrupt
	RTC->CR &= ~RTC_CR_WUTE;			// Disable Wakeup Timer
	
	while(!(RTC->ISR & RTC_ISR_WUTWF));	// Wait until Wakeup timer configuration update allowed
	
	RTC->WUTR = 5;								// The counter decrements by 1 every pulse of the WUCKSEL
	
	RTC->CR &= ~RTC_CR_WUCKSEL;
	RTC->CR |= RTC_CR_WUCKSEL_2;			// Select ck_spre (1Hz)
	
	//RTC->CR |= RTC_CR_WUTIE;
	RTC->CR |= RTC_CR_WUTE;
	
	// Enable Write protection
	RTC->WPR = 0xFF;
}
