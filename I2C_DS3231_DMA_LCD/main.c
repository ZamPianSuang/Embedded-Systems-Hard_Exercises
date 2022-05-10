#include "I2C1_DMA.h"
#include "LCD_20x4_4bits.h"

#define SLAVE_ADDR	0x68											// 1101 000 (base address of DS3231)

static uint8_t raw_data[7];										// sec, min, hour, day, date, month, year
static uint8_t data[7];
// Note: Index zero shall always contains the starting memory address of slave
static uint8_t data_write[8] = {0,0,89,114,3,38,1,34};	// 12:59 PM (Tue) 1/26/2022 in BCD format //114
static volatile int finished = 0;

int read_finish(void);
void reset_finish(void);
uint8_t bcd_to_decimal(uint8_t x, uint8_t t);

int main(void)
{
	LCD_Init();
	I2C1_Init();
	I2C1_Rx_DMA_Init();
	I2C1_Tx_DMA_Init();
	
	Lcd_Set_Cursor(1,5);
	Lcd_Write_String("TESTING DS3231");
	
	I2C1_write(0x68, data_write, sizeof(data_write));
	while(read_finish() == 0);
	reset_finish();
	
	uint8_t i;
	
	while(1)
	{
		I2C1_Read(0x68, 0x00, raw_data, 7);
		while(read_finish() == 0);
		reset_finish();
		
		for (i = 0; i < 7; i++)
		{
			data[i] = bcd_to_decimal(raw_data[i], i);
		}
		
		Lcd_Set_Cursor(2,1);
		Lcd_Write_Int(data[2]);
		Lcd_Set_Cursor(2,4);
		Lcd_Write_Int(data[1]);
		Lcd_Set_Cursor(2,7);
		Lcd_Write_Int(data[0]);
	}
}
int read_finish()
{
	return finished;
}
void reset_finish()
{
	finished = 0;
}
uint8_t bcd_to_decimal(uint8_t x, uint8_t t) 
{
	if(t == 0 || t == 1)								// second, minute
		return 10*((x >> 4) & 7) + (x & 0xF);
	
	else if(t == 2 || t == 5)						// hour, month
		return 10*((x >> 4) & 1) + (x & 0xF);
	
	else if(t == 3)									// day
		return x & 7; 
	
	else if(t == 4)									// date
		return 10*((x >> 4) & 3) + (x & 0xF);
	
	else if(t == 6)									// year
		return 10*(x >> 4) + (x & 0xF);
	else
		return 0;
}

void DMA1_Stream5_IRQHandler(void)
{
	if((DMA1->HISR)&DMA_HISR_TCIF5)
	{
		// I2C finished receiving using DMA1_Stream5
		finished = 1;
		I2C1->CR1 |= I2C_CR1_STOP;
		DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
	}
}

void DMA1_Stream6_IRQHandler(void)
{
	if((DMA1->HISR)&DMA_HISR_TCIF6)
	{
		/* I2C finished transmiting using DMA1_Stream6 */
		finished = 1;
		I2C1->CR1 |= I2C_CR1_STOP;
		DMA1->HIFCR |= DMA_HIFCR_CTCIF6;
	}
}
