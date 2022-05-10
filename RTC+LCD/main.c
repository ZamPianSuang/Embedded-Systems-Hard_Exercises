#include "RTC.h"
#include "LCD_20x4_4bits.h"

volatile static uint8_t hour, min, sec;
volatile static uint8_t day, month, year;
volatile static char *unit, *week;

int main(void)
{
	LCD_Init();
	Lcd_Set_Cursor(4,3);
	
	RTC_Init();
	RTC_Set_Date();
	
	Lcd_Write_String("CLOCK IS TICKING..");
	
	while(1)
	{
		Lcd_Set_Cursor(1,6);
		Get_Time(&hour, &min, &sec, &unit);
		
		Lcd_Write_Int(hour);
		Lcd_Write_Char(':');
		Lcd_Write_Int(min);
		Lcd_Write_Char(':');
		Lcd_Write_Int(sec);
		Lcd_Write_Char(' ');
		Lcd_Write_String(unit);
		
		Lcd_Set_Cursor(2,4);
		Get_Date(&day, &month, &year, &week);
		
		Lcd_Write_Int(day);
		Lcd_Write_Char('/');
		Lcd_Write_Int(month);
		Lcd_Write_Char('/');
		Lcd_Write_String("20");
		Lcd_Write_Int(year);
		Lcd_Write_Char(' ');
		Lcd_Write_String(week);
	}
}
