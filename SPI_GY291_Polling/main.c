/* Implementing ADXL345 or GY-291 Gyroscope Module */

#include "SPI.h"

#define DATA_FORMAT		0x31
#define OUT_X_L_ADDR		0x32							// 0x32-0x37 output registers

struct {
	float x;
	float y;
	float z;
}gyro;

static uint16_t gyro_x, gyro_y, gyro_z;
static uint8_t gyr[6], status;

int main(void)
{
	SPI1_Init();
	
	GYRO_IO_Read(&status, DATA_FORMAT, 1);			// Read DATA_FORMAT register		
	status &= ~(1U << 6);
	GYRO_IO_Write(&status, DATA_FORMAT, 1);		// Select 4-wire SPI mode	
	
	while(1)
	{
		// read 6 bytes from gyro
		GYRO_IO_Read(gyr, OUT_X_L_ADDR, 6);
		// Assume little endian (check control register 4 of gyro)
		gyro_x = (uint16_t) ((uint16_t)(gyr[1]<<8) + gyr[0]);
		gyro_y = (uint16_t) ((uint16_t)(gyr[3]<<8) + gyr[2]);
		gyro_z = (uint16_t) ((uint16_t)(gyr[5]<<8) + gyr[4]);
		// For +/- 2000dps, 1 unit equals to 70 millidegrees per second
		gyro.x = (float) gyro_x * 0x070f;			// X angular velocity
		gyro.y = (float) gyro_y * 0x070f;			// Y angular velocity
		gyro.z = (float) gyro_z * 0x070f;			// Z angular velocity
	}
}
