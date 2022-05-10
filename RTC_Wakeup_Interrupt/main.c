#include "RTC.h"

void Enter_SleepMode(void);
void systickDelayMs(int delay);

int main(void)
{
	LED_Init();
	RTC_Init();
	
	//
	
	RTC_Wakeup_Configuration();
	Enter_SleepMode();
	while(1)
	{
		

	}
}
void Enter_SleepMode(void)
{
	//PWR->CR &= ~PWR_CR_LPDS;		// Normal Sleep
	
	// RTC Wakeup interrupt is connected to EXTI 22 internaly
	EXTI->IMR |= EXTI_IMR_MR22;
	EXTI->EMR |= EXTI_EMR_MR22;
	EXTI->RTSR |= EXTI_RTSR_TR22;
	
	NVIC_SetPriority(RTC_WKUP_IRQn, 0);
	NVIC_EnableIRQ(RTC_WKUP_IRQn);
	
	//__DSB();								// Ensure that the last store takes effect
	//__WFI();								// Switch processor into the sleep mode
}
void systickDelayMs(int delay)	// see User Guide Note: LOAD is automatically reload again n again
{
	// Any value between 0x00000001 - 0x00FFFFFF (2^24 - 1)
	SysTick->LOAD = 16000-1; 			// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;					// Clear Current Value Register
	SysTick->CTRL |= 0x05;				// 101 - 1:processor clock, 1:counter enabled
										// 0:counting down to zero does not assert the SysTick exception request
	
	for (int i = 0; i < delay; i++)		//[16]Returns 1 if timer counted to 0 since last time this was read. 
	{
		// Wait until the COUNT flag is set
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	
	SysTick->CTRL = 0;					// stop the timer
}
