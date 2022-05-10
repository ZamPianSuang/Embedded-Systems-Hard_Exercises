#include "I2C.h"

uint8_t volatile i2c_buff = 0;

void I2C1_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;								// Enable I2C1 Clock
	
	I2C1->CR2 |= 16;														// FREQ[5:0] == APB clock frequency value
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
void DMA_Init(void)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;								// Enable DMA1 clock
	I2C1->CR2 |= I2C_CR2_DMAEN;										// DMA request enabled when TxE=1 or RxNE =1
	
	DMA1_Stream5->CR &= ~(3U << 6);						// Peripherial to Memory
	DMA1_Stream5->CR |= 1 << 8;							// Circular mode
	DMA1_Stream5->CR &= ~(1U << 9);						// Disable peripherial increment mode
	DMA1_Stream5->CR |= 1 << 10;							// Enable Memory increment mode
	DMA1_Stream5->CR &= ~((3U << 11) | (3U << 13));	// 8-bit (1 byte) for both Memory & Peripherial data sizes
	DMA1_Stream5->CR |= 1 << 25;							// Channel 1 selected
	
	DMA1_Stream5->CR |= 1 << 4;							// Enable TCIE interrupt
	//DMA1_Stream5->CR |= 1 << 0;							// Enable DMA stream
}

void I2C_Read(uint8_t devAdd, uint8_t len)
{
	// Using DMA to read data from slave
	uint32_t temp;
	
	DMA1_Stream5->NDTR = len;									// the Size of the transfer
	DMA1_Stream5->PAR  = (uint32_t) i2c_buff;		// Source address is Peripheral address
	DMA1_Stream5->M0AR = (uint32_t) &I2C1->DR;		// Destination address is Memory address
	
	DMA1_Stream5->CR |= 1 << 0;							// Enable DMA stream
	
	/* Read Byte Format Procedure */
	// S
	I2C1->CR1 |= I2C_CR1_START;							// Start generation
	while(!(I2C1->SR1 & I2C_SR1_SB));					// Wait for start bit flag
	// Address + WR
	I2C1->DR = (devAdd << 1) + 0;							// send address // +1 for read access
	while(!(I2C1->SR1 & I2C_SR1_ADDR));					// Wait for Address sent flag
	temp = I2C1->SR2;											// Read SR1 and SR2 to clear ADDR flag
	// -ACK
	// Command
	I2C1->DR = 0x00;											// Command: Read Temperature
	// -ACK
	// S
	I2C1->CR1 |= I2C_CR1_START;							// Start generation
	while(!(I2C1->SR1 & I2C_SR1_SB));					// Wait for start bit flag
	// Address + RD
	I2C1->DR = (devAdd << 1) + 1;							// send address // +1 for read access
	while(!(I2C1->SR1 & I2C_SR1_ADDR));					// Wait for Address sent flag
	temp = I2C1->SR2;											// Read SR1 and SR2 to clear ADDR flag
	// ACK + Data
	
	while(!(DMA1->HISR & DMA_HISR_TCIF5));				// Wait for TCIF5 flag
	I2C1->CR1 |= I2C_CR1_STOP;								// Stop generation
}
void I2C1_Start(uint32_t devAdd, uint8_t size, uint8_t dir)
{
	// Direction = 0 : Master requests a write transfer
	// Direction = 1 : Master requests a read transfer
	uint32_t tmpreg = I2C1->CR2;
}
void I2C1_WaitLineIdle(void)
{
	// wait until I2C bus is ready
	while(I2C1->SR2 & I2C_SR2_BUSY);
}
