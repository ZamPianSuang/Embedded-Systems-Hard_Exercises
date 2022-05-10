#include "stm32f4xx.h"                  // Device header

void RTC_Init(void);

int main(void)
{
	RTC_Init();
	
	while(1)
	{

	}
}
void RTC_Init(void)
{
	// Enable write access to the backup domain
	if((RCC->APB1ENR & RCC_APB1ENR_PWREN) == 0)
	{
		RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Enable Power interface clock 
		// Short delay after enabling an RCC peripheral clock
		(void) RCC->APB1ENR;
	}
	// Select LSE as RTC clock source
	// RTC clock: (1) LSE is in backup domain (2) HSE and LSI are not
	if((PWR->CR & PWR_CR_DBP) == 0)
	{
		// Enable access to RTC and RTC Backup registers and backup SRAM 
		PWR->CR |= PWR_CR_DBP;
		// Wait until backup domain write protection has been disabled
		while(!(PWR->CR & PWR_CR_DBP));
	}
	// Reset LSEON and LSEBYP bits before configuring LSE
	RCC->BDCR &= ~(RCC_BDCR_LSEON | RCC_BDCR_LSEBYP);
	
	// RTC clock selection can be changed only if the backup domain is reset
	RCC->BDCR |= RCC_BDCR_BDRST;
	RCC->BDCR &= ~RCC_BDCR_BDRST;
	
	// Enable LSE and wait for ready
	RCC->BDCR |= RCC_BDCR_LSEON;
	RCC->BDCR |= RCC_BDCR_LSERDY;
	
	// LSE oscillator clock used as the RTC clock
	RCC->BDCR &= ~RCC_BDCR_RTCSEL;
	RCC->BDCR |= RCC_BDCR_RTCSEL_0;
	
	RCC->APB1ENR |= RCC_APB1ENR_PWREN; // Disable Power interface clock 
	
	// Enable the clock of RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;
	
	// Disable Write protection
	RTC->WPR = 0xCA;
	RTC->WPR = 0x53;
	
	// Enter Init Mode
	RTC->ISR |= RTC_ISR_INIT;
	while(!(RTC->ISR & RTC_ISR_INITF));
	
	// Synchronous and Asynchronous prescaler
	RTC->PRER |= 127 << 16;
	RTC->PRER |= 255 << 0;
	
	// Hour format: 0 = 24 hour/day; 1 = AM/PM hour
	RTC->CR &= ~RTC_CR_FMT;		// 24 hour/day format
	
	// Set time as 11:32:00 am
	RTC->TR = 0 << 22 | 1 << 20 | 1 << 16 | 3 << 12 | 2 << 8 | 0 << 4 | 0 << 0;
	
	// Set date as 2021/05/27
	RTC->DR = 2 << 20 | 1 << 16 | 5 << 13 | 1 << 12 | 2 << 8 | 1 << 4 | 1 << 0;
	
	// Exit Initialization mode
	RTC->ISR &= ~RTC_ISR_INIT;
	
	// Enable Write protection
	RTC->WPR = 0xFF;
}
