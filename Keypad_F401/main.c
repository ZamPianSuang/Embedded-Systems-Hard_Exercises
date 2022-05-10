
#include "keypad_3x4.h"
#include "LCD_20x4_4bits.h"

static unsigned char key = 0;

int main(void)
{
	LCD_Init();
	keypad_Init();
	
	Lcd_Set_Cursor(1,4);
	Lcd_Write_String("KEYPAD TESTING");
	
	
	while(1)
	{
		Lcd_Set_Cursor(4,1);
		key = keypad_scan();
		Lcd_Write_Char(key);
		delayMs(10);
	}
}
