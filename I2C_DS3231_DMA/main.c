#include "I2C1_DMA.h"

#define SLAVE_ADDR	0x68										// 1101 000 (base address of DS3231)

static uint8_t data[7];											// sec, min, hour, day, date, month, year
static uint8_t data_r[7], data_s[4];
// Note: Index zero shall always contains the starting memory address
static uint8_t data_write[8] = {0,0,45,9,1,1,23,22};	
static volatile int finished = 0;

int read_finish(void);
void reset_finish(void);
uint8_t bcd_to_decimal(uint8_t x);

int main(void)
{
	I2C1_Init();
	I2C1_Rx_DMA_Init();
	I2C1_Tx_DMA_Init();
	
	I2C1_write(0x68, data_write, sizeof(data_write)+1);
	while(read_finish() == 0);
	reset_finish();
	
	while(1)
	{
		I2C1_Read(0x68, 0x00, data, 7);
		while(read_finish() == 0);
		reset_finish();
		for (int i = 0; i < 7; i++)
		{
			data_r[i] = bcd_to_decimal(data[i]);
		}
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
uint8_t bcd_to_decimal(uint8_t x) 
{
    return x - 6 * (x >> 4);
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
