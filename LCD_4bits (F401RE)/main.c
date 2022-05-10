#include "stm32f4xx.h"                  // Device header

#include "LCD_20x4_4bits.h"

int main(void)
{
	LCD_Init();
	
	while(1)
	{
		Lcd_Set_Cursor(1,1);
		Lcd_Write_String("LCD back again");
		Lcd_Set_Cursor(2,5);
		//Lcd_Shift_Right();
		Lcd_Write_String("WTF");
	}
}
