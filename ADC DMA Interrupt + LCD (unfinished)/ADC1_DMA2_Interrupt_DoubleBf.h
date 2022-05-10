#include "stm32f4xx.h"                  // Device header

void ADC_Init(void);
void ADC_Enable(void);
void ADC_Disable(void);
void ADC_Start(void);
uint32_t ADC_GetVal(void);
void DMA_Init(void);
void DMA_Config(uint32_t srcAdd, uint32_t destAdd, uint16_t size);
void DMA2_Stream0_IRQHandler(void);
void LED_Init(void);
