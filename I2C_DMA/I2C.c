#include "I2C.h"

uint8_t i2c_buff[10];

void I2C1_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;								// Enable I2C1 Clock
	
	I2C1->CR2 |= 16;														// FREQ[5:0] = APB clock frequency value
	I2C1->CCR &= ~I2C_CCR_FS;											// Sm mode I2C	
	/* Thigh = CCR * TPCLK1 & Tlow = CCR * TPCLK1
	If FREQ = 16, TPCLK = 62.5 ns 
	To generate 100 kHz, Thigh = Tlow = 50% each = (Period of 100 kHz / 2) = 5us
	CCR = Thigh/TPCLK = 5us / 62.5 ns = 80 */
	I2C1->CCR  |= 80;														// 50% duty cycle for each high and low
	I2C1->TRISE = 17;														// (1000 ns / TPCLK) + 1 = 16+1 
	I2C1->CR1  |= I2C_CR1_ACK;											// Enable Acks
	I2C1->CR1  &= ~I2C_CR1_NOSTRETCH;								// Enable Clock stretching
	I2C1->OAR1 &= ~I2C_OAR1_ADDMODE;									// 7-bit addressing mode
	
	/*************** GPIO ************/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;							// Enable GPIOB Clock
	GPIOB->MODER &= ~(GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	GPIOB->MODER |= GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1;	// PB6 & PB7 Alternate function mode
	GPIOB->OTYPER |= GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7;		// PB6 & PB7 Output open-drain
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7);	
	GPIOB->PUPDR |= GPIO_PUPDR_PUPD6_0 | GPIO_PUPDR_PUPD7_0;	// PB6 & PB7 Internal Pull-up (it might not enough)
	GPIOB->AFR[0] |= GPIO_AFRL_AFRL6_2 | GPIO_AFRL_AFRL7_2;	// PB6 & PB7 AF4
	
	I2C1->CR1 |= I2C_CR1_PE;											// Enable I2C1
}
void I2C_Write(uint8_t devAdd, uint8_t memAdd, uint8_t data)
{
	uint32_t temp;
	I2C1->CR1 |= I2C_CR1_START;										// Start generation
	while(!(I2C1->SR1 & I2C_SR1_SB));								// Wait for start condition
	I2C1->DR = devAdd;													// Send device address
	while(!(I2C1->SR1 & I2C_SR1_ADDR));								// Wait for End of address transmission
	temp = I2C1->SR2;														// Reading SR1 & SR2 to clear ADDR flag
	I2C2->DR = memAdd;													// Send device's memory address
	while(!(I2C1->SR1 & I2C_SR1_TXE));								// Wait for Data register empty
	/* add for loop for multiple byte sending */
	I2C1->DR = data;														// Data to write in that dev' mem address
	while(!(I2C1->SR1 & I2C_SR1_TXE));								// Wait for Data register empty
	/* loop */
	I2C1->CR1 |= I2C_CR1_STOP;											// Stop generation
}
void I2C_Read(uint8_t devAdd, uint8_t len)
{
	// Using DMA to read data from slave
	uint32_t temp;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;								// Enable DMA1 clock
	I2C1->CR2 |= I2C_CR2_DMAEN;										// DMA request enabled when TxE=1 or RxNE =1
	I2C1->CR1 |= I2C_CR1_ACK;											// Enable Acks
	
	/******************** DMA ********************/
	DMA1_Stream5->CR &= ~(3U << 6);						// Peripherial to Memory
	DMA1_Stream5->CR |= 1 << 8;							// Circular mode
	DMA1_Stream5->CR &= ~(1U << 9);						// Disable peripherial increment mode
	DMA1_Stream5->CR |= 1 << 10;							// Enable Memory increment mode
	DMA1_Stream5->CR &= ~((3U << 11) | (3U << 13));	// 8-bit (1 byte) for both Memory & Peripherial data sizes
	DMA1_Stream5->CR |= 1 << 25;							// Channel 1 selected
	
	DMA1_Stream5->NDTR = len;								// the Size of the transfer
	DMA1_Stream5->PAR  = (uint32_t) &I2C1->DR;		// Source address is Peripheral address
	DMA1_Stream5->M0AR = (uint32_t) i2c_buff;			// Destination address is Memory address
	
	DMA1_Stream5->CR |= 1 << 4;							// Enable TCIE interrupt
	DMA1_Stream5->CR |= 1 << 0;							// Enable DMA stream
	/******************** DMA ********************/
	
	I2C1->CR1 |= I2C_CR1_START;							// Start generation
	
	while(!(I2C1->SR1 & I2C_SR1_SB));					// Wait for start bit flag
	I2C1->DR = devAdd + 1;									// write 0xA0 // send address // +1 for read access
	while(!(I2C1->SR1 & I2C_SR1_ADDR));					// Wait for Address sent flag
	
	temp = I2C1->SR2;											// Read SR1 and SR2 to clear ADDR flag
	while(!(DMA1->HISR & DMA_HISR_TCIF5));				// Wait for TCIF5 flag
	I2C1->CR1 |= I2C_CR1_STOP;								// Stop generation
}

void delayMs(int delay)			// see User Guide Note: LOAD is automatically reload again n again
{
	// Any value between 0x00000001 - 0x00FFFFFF (2^24 - 1)
	SysTick->LOAD = 16000-1; 				// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;							// Clear Current Value Register
	SysTick->CTRL |= 0x05;					// 101 - 1:processor clock, 1:counter enabled
													// 0:counting down to zero does not assert the SysTick exception request
	
	for (int i = 0; i < delay; i++)		//[16]Returns 1 if timer counted to 0 since last time this was read. 
	{
		// Wait until the COUNT flag is set
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	
	SysTick->CTRL = 0;						// stop the timer
}
