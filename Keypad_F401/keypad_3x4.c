#include "keypad_3x4.h"

static unsigned char key_map [4][3] = {
	{'1', '2', '3' },
	{'4', '5', '6' },
	{'7', '8', '9' },
	{'*', '0', '#' },
};

void keypad_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	
	// PC0 - PC2 <-- Input Column
	GPIOC->MODER &= ~0x3FU;
	GPIOC->PUPDR &= ~0x3FU;
	
	// PC8 - PC11 --> Output Row
	GPIOC->MODER &= ~(0xFFU << 16);
	GPIOC->MODER |= 0x55 << 16;
	GPIOC->OTYPER &= ~(0xFU << 8);
	GPIOC->OSPEEDR |= 0xAA << 16;		// Fast speed
	GPIOC->PUPDR &= ~(0xFFU << 16);
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
	unsigned char row, col, ColumnPressed=0;
	unsigned char key = 0xFF;
	
	// Identify the row of the column pressed
	//for(row = 0; row < 4; row++)
	//{
		// Set up the row outputs to : 0111, 1011, 1101, 1110
	//	GPIOC->ODR &= ~((1U << 8) << row);		// set row output to low in turn		
	//	delayms(1);											// short delay
		
		for(col = 0; col < 3; col++)
		{
			if((GPIOC->IDR & (1 << col)) == 0)
			{
				delayms(1);
				if((GPIOC->IDR & (1 << col)) == 0)
				{
					ColumnPressed = col;
					//return findKey(row, ColumnPressed); 
					return 'K';
				}
			}
			delayms(1);
			//else
			//	return 'F';
		}	
		
	//	delayms(5);											// short delay
	//	GPIOC->ODR |= (1U << 8) << row;		// set row high to low in turn	
	//}	
}
void delayms(int delay)	// see User Guide Note: LOAD is automatically reload again n again
{
	SysTick->LOAD = 16000-1; 						// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;				
	SysTick->CTRL &= ~(0x10007U);		
	SysTick->CTRL |= 0x05;				
										
	for (int i = 0; i < delay; i++)	 
	{
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;					
}


