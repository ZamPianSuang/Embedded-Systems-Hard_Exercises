#include "stm32f4xx.h"                  // Device header
#include "RTC.h"

int main(void)
{	
	__disable_irq();
	
	LED_Init();
	RTC_Init();
	RTC_Set_Date();
	RTC_Set_Alarm();
	RTC_Alarm_Enable();
	
	__enable_irq();
	
	while(1)
	{

	} 
}
