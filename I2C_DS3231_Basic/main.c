#include "wire.h"

#define SLAVE_ADDR	0x68		// 1101 000

uint8_t time[3];					// second, minute, hour
uint8_t date[4];					// day, date, month, year

int main(void)
{
	I2C1_Init();
	
	while(1)
	{
		I2C_readbyte(SLAVE_ADDR, 0, &time[0]); 
	}
}
