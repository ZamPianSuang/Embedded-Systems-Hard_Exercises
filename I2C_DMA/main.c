#include "I2C.h"
#define EEPROM_ADDRESS 0xA0		// 0b10100000

int main(void)
{
	I2C1_Init();
	
	I2C_Write(EEPROM_ADDRESS, 0x04, 0x60); delayMs(2);
	I2C_Write(EEPROM_ADDRESS, 0x05, 0x61); delayMs(2);
	I2C_Write(EEPROM_ADDRESS, 0x06, 0x62); delayMs(2);
	I2C_Write(EEPROM_ADDRESS, 0x07, 0x63); delayMs(2); 
	I2C_Write(EEPROM_ADDRESS, 0x08, 0x64); delayMs(2);
	I2C_Write(EEPROM_ADDRESS, 0x03, 0x65); delayMs(2);
	
	I2C_Read(EEPROM_ADDRESS, 5);
	
	while(1)
	{
		
	}
}
