#include "stm32f4xx.h"                  // Device header

// 5 x 8 bits
// PC4 - PC7 : Data lines
// PB5 : RS
// PB7 : EN
// GND : RW

#define DB4 4
#define DB5 5
#define DB6 6
#define DB7 7

#define RS	5
#define EN	7

void GPIO_Init(void);
void Lcd_Port(int a);
void Lcd_Cmd(int a);
void LCD_Init(void);
void LCD_Clear(void);
void Lcd_Set_Cursor(char a, char b);
void Lcd_Write_Char(char a);
void Lcd_Write_Int(unsigned int a);
void Lcd_Write_String(volatile char *a);
void Lcd_Shift_Right(void);
void Lcd_Shift_Left(void);
void delayMs(int delay);
