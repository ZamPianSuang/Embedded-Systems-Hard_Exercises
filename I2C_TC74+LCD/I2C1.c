#include "I2C1.h"

uint8_t i2c_buff = 99;

void I2C1_Init(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;								// Enable I2C1 Clock
	
	I2C1->CR2 |= 16;														// FREQ[5:0] == APB clock frequency value
	I2C1->CCR &= ~I2C_CCR_FS;											// Sm mode I2C	
	/* Thigh = CCR * TPCLK1 & Tlow = CCR * TPCLK1
	 * If FREQ = 16, TPCLK = 62.5 ns 
	 * To generate 100 kHz, Thigh = Tlow = 50% each = (Period of 100 kHz / 2) = 5us
	 * CCR = Thigh/TPCLK = 5us / 62.5 ns = 80 */
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
	
	I2C1->CR2 |= I2C_CR2_DMAEN;										// DMA request enabled when TxE=1 or RxNE =1
	I2C1->CR1 |= I2C_CR1_PE;											// Enable I2C1
}
void DMA_Configure_I2C1_Rx(uint8_t *pRxBuffer, uint32_t size)
{
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;								// Enable DMA1 clock
	
	DMA1_Stream5->CR &= ~(1 << 0);									// Disable DMA stream
	DMA1_Stream5->CR |= 1 << 8;										// Circular mode
	DMA1_Stream5->CR &= ~(1U << 9);									// Disable peripherial increment mode
	DMA1_Stream5->CR |= 1 << 10;										// Enable Memory increment mode
	DMA1_Stream5->CR &= ~((3U << 11) | (3U << 13));				// 8-bit (1 byte) for both Memory & Peripherial data sizes
	DMA1_Stream5->CR &= ~(3U << 6);									// Peripherial to Memory
	DMA1_Stream5->CR |= 1 << 25;										// Channel 1 selected
	
	DMA1_Stream5->NDTR = size;											// the Size of the transfer
	DMA1_Stream5->PAR  = (uint32_t) &I2C1->DR;					// Source - Peripheral address
	DMA1_Stream5->M0AR = (uint32_t) pRxBuffer;					// Destination - Transmit buffer address
	
	DMA1_Stream5->CR |= 1 << 4;										// Enable TCIE interrupt
	DMA1_Stream5->CR |= 1 << 0;										// Enable DMA stream
}
void I2C1_WaitLineIdle(void)
{
	// wait until I2C bus is ready
	while((I2C1->SR2 & I2C_SR2_BUSY) == I2C_SR2_BUSY);
}
void I2C1_Start(uint32_t devAdd, uint8_t size, uint8_t direction)
{
	uint32_t temp;
	// Direction = 0 : Master requests a write transfer
	// Direction = 1 : Master requests a read transfer
	
	I2C1->CR1 |= I2C_CR1_START;							// Start generation
	//while(!(I2C1->SR1 & I2C_SR1_SB));					// Wait for start bit flag
	
	I2C1->DR = (devAdd << 1) + direction;				// send address // +1 for read access
	//while(!(I2C1->SR1 & I2C_SR1_ADDR));					// Wait for Address sent flag
	//temp = I2C1->SR2;											// Read SR1 and SR2 to clear ADDR flag
	
	//while(!(I2C1->SR1 & I2C_SR1_RXNE));
	//while(!(DMA1->HISR & DMA_HISR_TCIF5));				// Wait for TCIF5 flag
	
	I2C1->CR1 |= I2C_CR1_STOP;								// Stop generation
}
void I2C1_ReceiveData(uint8_t *pRxBuffer, uint8_t SlaveAddr, uint32_t size)
{
	// DMA must be initialized before setting the START bit
	DMA_Configure_I2C1_Rx(pRxBuffer, size);						// Configure DMA1 channel 1
	//I2C1_WaitLineIdle();													// Wait until I2C is available
	I2C1_Start(SlaveAddr, size, 1);									// 1 = receiving from the slave
}
/***
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
	// add for loop for multiple byte sending 
	I2C1->DR = data;														// Data to write in that dev' mem address
	while(!(I2C1->SR1 & I2C_SR1_TXE));								// Wait for Data register empty
	// loop 
	I2C1->CR1 |= I2C_CR1_STOP;											// Stop generation
}
***/
