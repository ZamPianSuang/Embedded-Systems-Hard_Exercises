#include "keypad_3x4.h"

void keypad_Init(void)
{
	/* Init PA0 - PA2 as Input ports */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	GPIOA->MODER &= ~0x3FU;			// PA0-PA2 Input mode
	//GPIOA->OTYPER &= ~0x7U;			// Output push-pull 
	//GPIOA->OSPEEDR &= ~0x3FU; 
	//GPIOA->OSPEEDR |= 0x3F;				// High speed
	GPIOA->PUPDR &= ~0x3FU;			// No pull-up, pull-down

	/* Init PB12 - PB15 as Output ports */
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	GPIOB->MODER &= ~(0xFFU << 24);
	GPIOB->MODER |= 0x55 << 24;				// PB0 - PB3 Output mode
	//GPIOB->OTYPER &= ~(0xFU << 12);			// Output push-pull 
	//GPIOB->OSPEEDR |= 0xFFU << 24;			// High speed
	GPIOB->PUPDR &= ~(0xFFU << 24);			// No pull-up, pull-down
}
unsigned char findKey(unsigned int row, unsigned int col)
{
   if(col == 0)
   {
		if(row == 0)
			return '1'; 
		else if(row == 1)
			return '4'; 
		else if(row == 2)
			return '7';
		else if(row == 2)
			return '*';
   }
   else if(col == 1)
   {
		if(row == 0)
			return '2'; 
		else if(row == 1) 
			return '5';
		else if(row== 2)
			return '8';
		else if(row == 2)
			return '0';
    }
   else if(col == 2)
    {
		if(row == 0) 
			return '3';
		else if(row == 1)
			return '6';
		else if(row == 2) 
			return '9'; 
		else if(row == 2)
			return '#';
    }
}
unsigned char keypad_scan(void)
{
	for(unsigned int i = 0; i < 4; i++)
	{
		/* send signal to output (row) ports individually */
		GPIOB->ODR |= (1U << 12) << i;			// row 1
		DelayMs(3);
		
		/* read signal from input (column) ports */
		
		if(GPIOA->IDR & GPIO_IDR_ID0)		// col 1
		{
			//while(GPIOA->IDR & GPIO_IDR_ID0);
			return 'K';
			//return findKey(i, 0);
		}
		else if(GPIOA->IDR & GPIO_IDR_ID1)		// col 2
		{

			//while(GPIOA->IDR & GPIO_IDR_ID1);
			//return findKey(i, 1);
			return 'L';
		}
		else if(GPIOA->IDR & GPIO_IDR_ID2)		// col 3
		{
			//while(GPIOA->IDR & GPIO_IDR_ID2);
			//return findKey(i, 2);
			return 2;
		}
		
		DelayMs(5);
		GPIOB->ODR &= ~(1U << 12) << i;			// row 1
	}
}

void DelayMs(int delay)	// see User Guide Note: LOAD is automatically reload again n again
{
	SysTick->LOAD = 16000-1; 						// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;					
	SysTick->CTRL |= 0x05;				
										
	for (int i = 0; i < delay; i++)	 
	{
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;					
}
