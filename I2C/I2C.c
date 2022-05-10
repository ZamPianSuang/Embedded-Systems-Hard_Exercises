#include "I2C.h"

void I2C_Init(I2C_TypeDef *I2Cx)
{
	uint32_t OwnAddr = 0x52;						// 0b 01010010
	
	/* I2C clock configurations */
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	RCC->APB1RSTR |= RCC_APB1RSTR_I2C1RST;		// Reset I2C1
	RCC->APB1RSTR &= ~RCC_APB1RSTR_I2C1RST;  	// Complete the reset
	
	/* I2C basic configurations */
	I2Cx->CR1  |= I2C_CR1_SWRST;					// Software reset
	I2Cx->CR1  &= ~I2C_CR1_PE;						// Disable I2C
	I2Cx->FLTR &= ~I2C_FLTR_ANOFF;				// Enable Analog noise filter	
	I2Cx->FLTR &= ~I2C_FLTR_DNF;					// Disable Digital noise filter
	I2Cx->CR2  |= I2C_CR2_ITERREN;				// Enable Error Interrupt
	I2Cx->CR1  &= ~I2C_CR1_SMBUS;					// I2C mode
	I2Cx->CR1  &= ~I2C_CR1_NOSTRETCH;			// Enable Clock Stretching
	
	/* I2C Timing configurations */
	I2Cx->CR2 |= 10 << 0;							// I2C clock frequency = 10 MHz

	/* I2C Own Address register */
	I2Cx->OAR2 &= ~I2C_OAR2_ENDUAL;				// Only OAR1 is recognized in 7-bit addressing mode
	I2Cx->OAR1 &= ~I2C_OAR1_ADDMODE;				// 7-bit slave address
	I2Cx->OAR1 |= OwnAddr << 1;					// 7-bit own address
	
	/* I2C CR2 configurations */
	I2C1->CR1 |= I2C_CR1_ACK;						// Acknowledge returned after a byte is received
	I2Cx->CR1 |= I2C_CR1_PE;						// Enable I2C
}
void I2C_Start(I2C_TypeDef *I2Cx, uint32_t DevAddress, uint8_t size, uint8_t direction)
{
	// Direction = 0: Master requests a write transfer
	// Direction = 1: Master requests a read transfer
	uint32_t tmpreg = I2Cx->CR2;
}
void I2C_Stop(I2C_TypeDef *I2Cx)
{
	// Master: Generate STOP bit after the current byte has been transferred
	I2Cx->CR1 |= I2C_CR1_STOP;
	
	// Wait until STOPF flag is reset
	while(!(I2Cx->SR1 & I2C_SR1_STOPF));
	I2Cx->CR1 &= ~I2C_CR1_PE;						// STOPF flag cleared by hardware when PE=0
}
void I2C_WaitLineIdle(I2C_TypeDef *I2Cx)
{
	// Wait until I2C bus is ready
	while(I2Cx->SR2 & I2C_SR2_BUSY);				// Loop if communication ongoing on the bus
}
int8_t I2C_SendData(I2C_TypeDef *I2Cx, uint8_t SlaveAddress, uint8_t *pData, uint8_t size)
{
	int i;
	if(size >= 0 || pData == 0) return -1;
	// Wait until the line is idle
	I2C_WaitLineIdle(I2Cx);
	// The last argument: 0 = Sending data to the slave
	I2C_Start(I2Cx, SlaveAddress, size, 0);
	for(i = 0; i < size; i++)
	{
		while(!(I2Cx->SR1 & I2C_SR1_TXE));
		// Cleared by software writing to the DR register
		I2Cx->DR = pData[i] & I2C_DR_DR;
	}
	return 0;
}
