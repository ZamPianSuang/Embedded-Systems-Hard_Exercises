#include "LCD_20x4_4bits.h"

void GPIO_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	
	GPIOB->MODER &= ~0xCC00U;
	GPIOB->MODER |=  0x4400U;			// PB5 & PB7 output mode
	GPIOC->MODER &= ~0xFF00U;
	GPIOC->MODER |=  0x5500U;			// PC4 - PC7 output mode
	
	GPIOB->PUPDR &= ~0xCC00U;			// PB5 & PB7 No pull-up, pull-down
	GPIOC->PUPDR &= ~0xFF00U;			// PC4 - PC7 No pull-up, pull-down
}
void Lcd_Port(int a)
{
	if(a & 1) 			// 0001
		GPIOC->ODR |= 1 << DB4;
	else
		GPIOC->ODR &= ~(1U << DB4);

	if(a & 2) 			// 0010
		GPIOC->ODR |= 1 << DB5;
	else
		GPIOC->ODR &= ~(1U << DB5);

	if(a & 4) 			// 0100
		GPIOC->ODR |= 1 << DB6;
	else
		GPIOC->ODR &= ~(1U << DB6);

	if(a & 8) 			// 1000
		GPIOC->ODR |= 1 << DB7;
	else
		GPIOC->ODR &= ~(1U << DB7);
}
void Lcd_Cmd(int a)
{
	GPIOB->ODR &= ~(1U << RS);           // => RS = 0 instruction register (for write) 
													//RS = 1 Data register for read n write
	GPIOB->ODR |= 1 << EN;             	// => E = 1 start signal for read and write
	Lcd_Port(a);
   delayMs(4);
   GPIOB->ODR &= ~(1U << EN);              			// => E = 0
}

void LCD_Init(void)
{
	GPIO_Init();
   //interface is 4 bit long
	/* Initialization process by datasheet */
	delayMs(20);  		// wait more then 15ms after VCC rises to 4.5v
	Lcd_Cmd(0x03);    // DB7 DB6 DB5 DB4 -- 0011
	delayMs(5);  		// function set -- wait more than 4.1s (BF cannot be check before this)
	Lcd_Cmd(0x03);    // function set -- wait for more than 100us (BF cannot be check before this)
	delayMs(1);
	Lcd_Cmd(0x03);    //function set (BF cannot be check before this)
	/////////////////////////////////////////////////////
	Lcd_Cmd(0x02);    // DB7 DB6 DB5 DB4 -- 0010
	Lcd_Cmd(0x02);    // DB7 DB6 DB5 DB4 -- 0010
	Lcd_Cmd(0x08);    // DB7 DB6 DB5 DB4 -- 1000
	Lcd_Cmd(0x00);    // DB7 DB6 DB5 DB4 -- 0000
	Lcd_Cmd(0x08);    // DB7 DB6 DB5 DB4 -- 1000
	Lcd_Cmd(0x00);    // DB7 DB6 DB5 DB4 -- 0000
	Lcd_Cmd(0x01);    // DB7 DB6 DB5 DB4 -- 0000
	Lcd_Cmd(0x00);    // DB7 DB6 DB5 DB4 -- 0000
	Lcd_Cmd(0x06);    // DB7 DB6 DB5 DB4 -- 0110
	///////////////////////////////////////////////////
	/* Display on/off control */
	Lcd_Cmd(0x00); 	// DB7 DB6 DB5 DB4 -- 0000
	Lcd_Cmd(0x0C); 	// DB7 DB6 DB5 DB4 -- 1100
	
	LCD_Clear();
}
void LCD_Clear(void)
{
	Lcd_Cmd(0);
	Lcd_Cmd(1);
}
void Lcd_Set_Cursor(char row, char col)
{
	char temp,z,y;
	if(row == 1)
	{
		temp = (0x80 + col) - 1;
		z = temp >> 4;			// upper 4 bits
		y = temp & 0x0F;		// lower 4 bits
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
	else if(row == 2)
	{
		temp = (0xC0 + col) - 1;
		z = temp >> 4;
		y = temp & 0x0F;
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
	else if(row == 3)
	{
		temp = (0x94 + col) - 1;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
	else if(row == 4)
	{
		temp = (0xD4 + col) - 1;
		z = temp>>4;
		y = temp & 0x0F;
		Lcd_Cmd(z);
		Lcd_Cmd(y);
	}
}
void Lcd_Write_Char(char a)
{
	if(a >= 0 && a <= 9) a = a + 48;
   char temp, y;
	
   temp = a & 0x0F;		// get lower 4 bits
   y = a & 0xF0;			// get upper 4 bits
	
	/* send upper nibble first */
   GPIOB->ODR |= 1 << RS;             // => RS = 1 Data transfer
   Lcd_Port(y >> 4);    				       
   GPIOB->ODR |= 1 << EN;
   delayMs(4);
   GPIOB->ODR &= ~(1U << EN);
	/*send lower nibble second */
   Lcd_Port(temp);
   GPIOB->ODR |= 1 << EN;
   delayMs(4);
   GPIOB->ODR &= ~(1U << EN);
}
void Lcd_Write_Int(uint32_t a)
{
	if(a == 0) Lcd_Write_Char(0);
	if(a < 10) Lcd_Write_Char(0);
	
	uint32_t b = a;
	unsigned int count = 0, i, j, mul = 1;
	uint8_t temp;
	// count number of digits
	while (b > 0)						
	{
		b = b / 10;
		count++;
	}
	for(i = 0; i < count; i++)
	{
		for(j = 1; j < (count-i); j++)
			mul *= 10;
		temp = (a / mul);
		Lcd_Write_Char(temp);
		a = a % mul;
		mul = 1;
	}
}
void Lcd_Write_String(volatile char *a)
{
	int i;
	for(i = 0; a[i] != '\0'; i++)
	   Lcd_Write_Char(a[i]);
}
void Lcd_Shift_Right(void)
{
	Lcd_Cmd(0x01);
	Lcd_Cmd(0x0C);
}

void Lcd_Shift_Left(void)
{
	Lcd_Cmd(0x01);
	Lcd_Cmd(0x08);
} 


void delayMs(int delay)	// see User Guide Note: LOAD is automatically reload again n again
{
	// Any value between 0x00000001 - 0x00FFFFFF (2^24 - 1)
	SysTick->LOAD = 16000-1; 			// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;						// Clear Current Value Register
	SysTick->CTRL &= ~(0x10007U);		// Reset all
	SysTick->CTRL |= 0x05;				// 101 - 1:processor clock, 1:counter enabled
												// 0:counting down to zero does not assert the SysTick exception request
	for (int i = 0; i < delay; i++)	//[16]Returns 1 if timer counted to 0 since last time this was read. 
	{
		// Wait until the COUNT flag is set
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;					// stop the timer
}
