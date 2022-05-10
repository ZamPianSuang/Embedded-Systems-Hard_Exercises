#include "ADC1_DMA2.h"
#include "LCD_20x4_4bits.h"

static volatile uint16_t RxData[2];

void System_Clock_Init(void);
void LED_Init(void);

int main(void)
{
	System_Clock_Init();			// 80 MHz
	LED_Init();
	
	LCD_Init();
	Lcd_Set_Cursor(1,7);
	Lcd_Write_String("DMA TEST");
	Lcd_Set_Cursor(2,1);
	Lcd_Write_String("PA1 - ");
	Lcd_Set_Cursor(3,1);
	Lcd_Write_String("PA4 - ");
	
	ADC_Init();
	ADC_Enable();
	DMA_Init();
	
	DMA_Config((uint32_t)&ADC1->DR, (uint32_t) RxData, 2);
	
	ADC_Start();
	
	while(1)
	{
		Lcd_Set_Cursor(2,7);
		Lcd_Write_Int(RxData[0]);
		Lcd_Set_Cursor(3,7);
		Lcd_Write_Int(RxData[1]);
		GPIOA->ODR ^= 1 << 5;
		delayMs(100);
	}
}
void System_Clock_Init(void)
{
	/* SYSCLK - 80 MHz */
	// (1) Enable clock source and wait for ready
	RCC->CR |= RCC_CR_HSION;
	while(!(RCC->CR & RCC_CR_HSIRDY)); 
	
	// (2) Set PWREN and VOS 
	// Set Power Enable Clock and Voltage Regulator
	RCC->APB1ENR |= RCC_APB1ENR_PWREN;
	PWR->CR |= PWR_CR_VOS;						// Scale 1 mode (reset value)

	// (3) Configure Flash
	// Activate prefetch buffer but it should already be on
	FLASH->ACR |= FLASH_ACR_PRFTEN;
	// 3 Wait State (72 < HCLK = 96) 
	FLASH->ACR &= ~(FLASH_ACR_LATENCY);
	FLASH->ACR |= 0x3U;

	// (4) Configure Main PLL
	// Select HSI as PLL entry clock source
	RCC->PLLCFGR |= RCC_PLLCFGR_PLLSRC_HSI;
	// VCO input frequency = PLL input frequency / PLLM[5:0] (VCO = 8/8)
	RCC->PLLCFGR &= ~(0x3FU);					// clear RCC_PLLCFGR [5:0]
	RCC->PLLCFGR |= 8;							// VCO input = 16/8 = 2 MHz
	// VCO output frequency = VCO input frequency × PLLN[8:0]
	RCC->PLLCFGR &= ~(0x7FC0U);				// Clear RCC_PLLCFGR [14:6]
	RCC->PLLCFGR |= 80 << 6;					// VCO output = 2*80 = 160 MHz
	// PLL output clock frequency = VCO frequency / PLLP[1:0]
	RCC->PLLCFGR &= ~(3U << 16); 				// PLLCLK output = 160/2 = 80 MHz

	// (5) Configure prescalars
	RCC->CFGR &= ~(0xFU << 4);					// system clock not divided (AHB/1)
	RCC->CFGR &= ~(0x7U << 10);		
	RCC->CFGR |= RCC_CFGR_PPRE1_2;			// APB1 = AHB/2
	RCC->CFGR &= ~(0x7U << 13);				// APB2 = AHB/1

	// (7) Enable PLL and wait for ready
	RCC->CR |= RCC_CR_PLLON;
	while(!(RCC->CR & RCC_CR_PLLRDY));
	
	// (6) Select the clock source for system clock
	RCC->CFGR |= RCC_CFGR_SW_1;				// PLL_P selected as system clock
	while(!(RCC->CFGR & RCC_CFGR_SWS_PLL));
	
	SystemCoreClockUpdate();
}
void LED_Init(void)
{
	// LED is in PA5 for STM32F446RE //
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;	// MAX 180 MHz //
	// Set PA5 as Output Function
	GPIOA->MODER &= ~(3U<<10);					// Clear bit 11 and 10
	GPIOA->MODER |= (1U<<10);		
	// PA5: Output push-pull
	GPIOA->OTYPER &= ~(1U<<5);
	// PA5: Fast speed
	GPIOA->OSPEEDR &= ~(3U<<10);				// Clear bit 11 and 10
	GPIOA->OSPEEDR |= (1U<<11);
	// PA5: No pull-up/pull-down
	GPIOA->PUPDR &= ~(3U<<10);
}
