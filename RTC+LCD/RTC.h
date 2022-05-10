#include "stm32f4xx.h"                  // Device header

void RTC_Init(void);
void RTC_Set_Date(void);
void Get_Time(volatile uint8_t *hour, volatile uint8_t *min, volatile uint8_t *sec, volatile char **unit);
void Get_Date(volatile uint8_t *day, volatile uint8_t *month, volatile uint8_t *year, volatile char **week);
