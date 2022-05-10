#include "RTC.h"

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
	RTC->CR |= RTC_CR_FMT;		// Using AM/PM format
	
	// Set time as 1:30:00 pm
	RTC->TR = 0 << 22 | 1 << 20 | 1 << 16 | 5 << 12 | 9 << 8 | 3 << 4 | 0 << 0;
	
	// Set date as 2021/(Thurs)/11/18 
	RTC->DR = 2 << 20 | 1 << 16 | 4 << 13 | 1 << 12 | 1 << 8 | 1 << 4 | 8 << 0;
	
	// Exit Initialization mode
	RTC->ISR &= ~RTC_ISR_INIT;
	
	// Enable Write protection
	RTC->WPR = 0xFF;
	
	// Enable the clock of RTC
	RCC->BDCR |= RCC_BDCR_RTCEN;
}
void Get_Time(volatile uint8_t *hour, volatile uint8_t *min, volatile uint8_t *sec, volatile char **unit)
{
	uint8_t T, U;
	uint32_t Full;
	
	while(!(RTC->ISR & RTC_ISR_RSF));
	Full = RTC->TR;
	
	if(Full & RTC_TR_PM)
		*unit = "PM";
	else
		*unit = "AM";
	
	T = (Full >> 20) & (RTC_TR_HT >> 20);
	U = (Full >> 16) & (RTC_TR_HU >> 16);
	*hour = (T * 10) + U;
	
	T = (Full >> 12) & (RTC_TR_MNT >> 12);
	U = (Full >> 8) & (RTC_TR_MNU >> 8);
	*min = (T * 10) + U;
	
	T = (Full >> 4) & (RTC_TR_ST >> 4);
	U = (Full >> 0) & (RTC_TR_SU >> 0);
	*sec = (T * 10) + U;
	
	RTC->ISR &= ~RTC_ISR_RSF;
}
void Get_Date(volatile uint8_t *day, volatile uint8_t *month, volatile uint8_t *year, volatile char **week)
{
	uint8_t T, U;
	uint32_t Full;
	
	while(!(RTC->ISR & RTC_ISR_RSF));
	Full = RTC->DR;
	
	T = (Full >> 4) & (RTC_DR_DT >> 4);
	U = (Full >> 0) & (RTC_DR_DU >> 0);
	*day = (T * 10) + U;
	
	T = (Full >> 12) & (RTC_DR_MT >> 12);
	U = (Full >> 8) & (RTC_DR_MU >> 8);
	*month = (T * 10) + U;
	
	T = (Full >> 20) & (RTC_DR_YT >> 20);
	U = (Full >> 16) & (RTC_DR_YU >> 16);
	*year = (T * 10) + U;
	
	T = (Full >> 13) & (RTC_DR_WDU >> 13);
	switch(T)
	{
		case 1:
				*week = "Mon"; break;
		case 2:
				*week = "Tue"; break;
		case 3:
				*week = "Wed"; break;
		case 4:
				*week = "Thurs"; break;
		case 5:
				*week = "Fri"; break;
		case 6:
				*week = "Sat"; break;
		case 7:
				*week = "Sun"; break;
	}
	
	RTC->ISR &= ~RTC_ISR_RSF;
}
