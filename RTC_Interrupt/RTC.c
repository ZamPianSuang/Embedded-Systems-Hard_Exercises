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
void RTC_Set_Date(void)
{
	// Disable Write protection
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	// Enter Init Mode
	RTC->ISR |= RTC_ISR_INIT;
	while(!(RTC->ISR & RTC_ISR_INITF));
	
	// Disable the clock of RTC
	RCC->BDCR &= ~RCC_BDCR_RTCEN;
	
	// Synchronous and Asynchronous prescaler
	RTC->PRER = 0;
	RTC->PRER |= 0x7F << 16;
	RTC->PRER |= 0xFF << 0;
	
	// Hour format: 0 = 24 hour/day; 1 = AM/PM hour
	RTC->CR |= RTC_CR_FMT;		// AM/PM format
	
	// Set time as 11:32:00 am
	RTC->TR = 0 << 22 | 1 << 20 | 1 << 16 | 3 << 12 | 2 << 8 | 0 << 4 | 0 << 0;
	
	// Set date as 2021/(Friday)/05/27 
	RTC->DR = 2 << 20 | 1 << 16 | 5 << 13 | 0 << 12 | 5 << 8 | 2 << 4 | 7 << 0;
	
	// Exit Initialization mode
	RTC->ISR &= ~RTC_ISR_INIT;
	
	// Enable Write protection
	RTC->WPR = 0xFF;
}
void RTC_Set_Alarm(void)
{
	uint32_t AlarmTimeRegA, AlarmTimeRegB;
	
	// Set off alarm A 30 and B 60
	AlarmTimeRegA = 3 << 4;				// Bits[6:4] = Ten's digit for the second
	AlarmTimeRegB = 0 << 4;				//  digit for the second
	
	// Disable Write protection
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	// Disable alarm A and it's interrupt
	RTC->CR &= ~RTC_CR_ALRAE;		
	RTC->CR &= ~RTC_CR_ALRAIE;
	// Wait until Alarm A update allowed
	while(!(RTC->ISR & RTC_ISR_ALRAWF));
	
	// Insert RTC alarm A register
	RTC->ALRMAR = AlarmTimeRegA;
	
	// Set alarm mask field to compare only the second for A
	RTC->ALRMAR |= RTC_ALRMAR_MSK4;		// Ignore day of week in comparison
	RTC->ALRMAR |= RTC_ALRMAR_MSK3;		// Ignore hour in comparison
	RTC->ALRMAR |= RTC_ALRMAR_MSK2;		// Ignore minute in alarm comparison
	RTC->ALRMAR &= ~RTC_ALRMAR_MSK1;		// Alarm sets off if the second match
	
	// Disable alarm B and it's interrupt
	RTC->CR &= ~RTC_CR_ALRBE;		
	RTC->CR &= ~RTC_CR_ALRBIE;
	// Wait until Alarm A update allowed
	while(!(RTC->ISR & RTC_ISR_ALRBWF));
	
	// Insert RTC alarm B register
	RTC->ALRMBR = AlarmTimeRegB;
	
	// Set alarm mask field to compare only the second for B
	RTC->ALRMBR |= RTC_ALRMBR_MSK4;		// Ignore day of week in comparison
	RTC->ALRMBR |= RTC_ALRMBR_MSK3;		// Ignore hour in comparison
	RTC->ALRMBR |= RTC_ALRMBR_MSK2;		// Ignore minute in alarm comparison
	RTC->ALRMBR &= ~RTC_ALRMBR_MSK1;		// Alarm sets off if the second match
	
	// Enable alarm A and it's interrupt
	RTC->CR |= RTC_CR_ALRAE; 						
	RTC->CR |= RTC_CR_ALRAIE; 			
	
	// Enable alarm B and it's interrupt
	RTC->CR |= RTC_CR_ALRBE; 					
	RTC->CR |= RTC_CR_ALRBIE; 	
	
	// Enable Write protection
	RTC->WPR = 0xFF;
}
void RTC_Alarm_Enable(void)
{
	// Rising trigger enabled (for Event and Interrupt) for input line
	EXTI->RTSR |= EXTI_RTSR_TR17;
	EXTI->FTSR &= ~EXTI_FTSR_TR17;
	// Interrupt request from line 17 is not masked
	EXTI->IMR |= EXTI_IMR_MR17;   // EXTI_IMR_IM17;
	// Event request from line x is not masked
	EXTI->EMR |= EXTI_EMR_MR17;
	
	//EXTI->SWIER &= ~EXTI_SWIER_SWIER17;
	//EXTI->PR |= EXTI_PR_PR17;
	
	// Set interrupt priority as the most urgent
	NVIC_SetPriority(RTC_Alarm_IRQn, 1);
	
	// Enable RTC interrupt
	NVIC_EnableIRQ(RTC_Alarm_IRQn);
	
	// Enable the clock of RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;
}
void RTC_Alarm_IRQHandler(void)
{
	if((RTC->ISR & RTC_ISR_ALRAF) == RTC_ISR_ALRAF)
	{
		GPIOA->ODR |= (1U << 5);

		RTC->ISR &= ~RTC_ISR_ALRAF;		// Clear the alarm A interrupt flag
		
	}
	if((RTC->ISR & RTC_ISR_ALRBF) == RTC_ISR_ALRBF)
	{
		GPIOA->ODR &= ~(1U << 5);
		
		RTC->ISR &= ~RTC_ISR_ALRBF;		// Clear the alarm A interrupt flag

	}
	// Clear the EXTI line 17
	EXTI->PR = EXTI_PR_PR17;
}
