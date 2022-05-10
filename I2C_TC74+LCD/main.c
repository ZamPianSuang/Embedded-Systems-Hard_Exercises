// Default address of TC74A5 - 1001 101 b - 0x4D

#include "I2C1.h"
#include "LCD_20x4_4bits.h"

#define ADDRESS 0x4D

int main(void)
{
	LCD_Init();
	Lcd_Set_Cursor(1,5);
	Lcd_Write_String("TESTING TC74");
	Lcd_Set_Cursor(2,1);
	Lcd_Write_String("TEMP: ");
	
	I2C1_Init();
	
	while(1)
	{
		I2C1_ReceiveData(&i2c_buff, ADDRESS, 1);				// Receive 1 byte from slave module
		Lcd_Set_Cursor(2,7);
		delayMs(100);
		Lcd_Write_Int((uint32_t)I2C1->DR);
	}
}
