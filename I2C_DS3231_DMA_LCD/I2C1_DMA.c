// I2C1_SCL - PB8 - AF4
// I2C1_SDA - PB9 - AF4
// DMA1_Stream5_Channel1 is I2C1_Rx
// DMA1_Stream6_Channel1 is I2C1_Tx

#include "I2C1_DMA.h"

void I2C1_Init(void)
{
	/***** Clock Initializations *****/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;			// Enable GPIOB clock
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;				// Enable I2C1 clock
	
	/***** GPIO Initializations *****/
	GPIOB->MODER &= ~(GPIO_MODER_MODE8 | GPIO_MODER_MODE9);
	GPIOB->MODER |= (GPIO_MODER_MODE8_1 | GPIO_MODER_MODE9_1);		// PB8 & PB9 as Alternate function mode
	
	GPIOB->AFR[1] &= ~(GPIO_AFRH_AFRH0 | GPIO_AFRH_AFRH1);
	GPIOB->AFR[1] |= (GPIO_AFRH_AFRH0_2 | GPIO_AFRH_AFRH1_2);		// PB8 & PB9 as AF4
	
	GPIOB->OTYPER |= (GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9);				// PB8 & PB9 as Open-drain
	
	GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD8 | GPIO_PUPDR_PUPD9);
	GPIOB->PUPDR |= (GPIO_PUPDR_PUPD8_0 | GPIO_PUPDR_PUPD9_0);		// PB8 & PB9 Pull-up
	
	/***** I2C Configurations *****/
	I2C1->CR1 |= I2C_CR1_SWRST;												// Software reset
	I2C1->CR1 &= ~I2C_CR1_SWRST;												// Unreset (procedure)
	I2C1->CR1 &= ~I2C_CR1_NOSTRETCH;											// Enable Clock Stretching
	I2C1->CR1 &= ~I2C_CR1_ENGC;												// General call disabled. Address 00h is NACKed
	
	I2C1->CR2 |= I2C_CR2_LAST;													// Next DMA EOT is not the last transfer
	I2C1->CR2 |= I2C_CR2_DMAEN;												// DMA request enable
	
	I2C1->CR2 &= ~I2C_CR2_FREQ;
	I2C1->CR2 |= 16;																// System Clock = 16 MHz
	
	/* Thigh = Tlow = CCR * Tcplk1 (see Reference Manual)
	 * If FREQ = 16, Tcplk1 = 62.5 ns
	 * T = 100 kHz (Standard Mode) = 10 us
	 * To achieve 50% duty cycle, Thigh = Tlow = T/2 = 5us
	 * That's why CCR = Thigh(or)Tlow / Tcplk1 = 80 */
	I2C1->CCR &= ~I2C_CCR_CCR;
	I2C1->CCR |= 80;
	
	/* TRISE = (1000ns / Tcplk) + 1 = 17 */
	I2C1->TRISE &= ~I2C_TRISE_TRISE;
	I2C1->TRISE |= 17;															// Max rise time
	
	I2C1->CR1 |= I2C_CR1_PE;													// Enable I2C1
}
void I2C1_Rx_DMA_Init(void)
{
	// DMA1_Stream5_Channel1 is I2C1_Rx
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	
	DMA1_Stream5->CR = 0x00;//reset everything
	while((DMA1_Stream5->CR) & DMA_SxCR_EN);								//  		
	
	DMA1_Stream5->CR |= DMA_SxCR_CHSEL_0;									// Channel 1 selected
	DMA1_Stream5->CR &= ~(DMA_SxCR_MSIZE | DMA_SxCR_PSIZE);			// 8-bits data for both Peripherial and Memory
	DMA1_Stream5->CR &= ~DMA_SxCR_DIR;										// Peripherial to Memory mode
	DMA1_Stream5->CR |= DMA_SxCR_MINC;										// Increment Memory address pointer
	DMA1_Stream5->CR |= DMA_SxCR_TCIE;										// Enable Transfer Complete Interrupt
	
	NVIC_EnableIRQ(DMA1_Stream5_IRQn);
}
void I2C1_Tx_DMA_Init(void)
{
	// DMA1_Stream6_Channel1 is I2C1_Tx
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;
	
	DMA1_Stream6->CR = 0;
	while((DMA1_Stream6->CR) & DMA_SxCR_EN);
	
	DMA1_Stream6->CR |= DMA_SxCR_CHSEL_0;									// Channel 1 selected
	DMA1_Stream6->CR &= ~(DMA_SxCR_MSIZE | DMA_SxCR_PSIZE);			// 8-bits data for both Peripherial and Memory
	DMA1_Stream6->CR |= DMA_SxCR_DIR_0;										// Memory to Peripherial mode
	DMA1_Stream6->CR |= DMA_SxCR_MINC;										// Increment Memory address pointer
	DMA1_Stream6->CR |= DMA_SxCR_TCIE;										// Enable Transfer Complete Interrupt
	
	NVIC_EnableIRQ(DMA1_Stream6_IRQn);
}
static void DMA_Transmit(const uint8_t *pBuffer, uint16_t size)	// static function is only visible in its object file
{
	if(NULL != pBuffer)												// Check for NULL pointers
	{
		DMA1_Stream6->CR &= ~DMA_SxCR_EN;						// Disable Stream6
		while((DMA1_Stream6->CR) & DMA_SxCR_EN);		
		
		/* Set Memory addresses and data items */
		DMA1_Stream6->M0AR = (uint32_t)pBuffer;
		DMA1_Stream6->PAR  = (uint32_t)&I2C1->DR;
		DMA1_Stream6->NDTR = size+1;								// have to add +1 when writing, idk wtf 
		
		/* Clear all Interrupt Flags */
		DMA1->HIFCR |= DMA_HIFCR_CTCIF6;
		
		DMA1_Stream6->CR |= DMA_SxCR_EN;							// Enable Stream6
	}
}
void I2C1_write(uint8_t SensorAddr, uint8_t *pWriteBuffer, uint16_t NumByteToWrite)
{
	while(I2C1->SR2 & I2C_SR2_BUSY);								// Wait while Communication ongoing on the bus
	
	I2C1->CR1 |= I2C_CR1_START;									// generate start condition
	while(!(I2C1->SR1 & I2C_SR1_SB));							// Wait for start bit flag
	(void)I2C1->SR1;													// Read SR1
	  
	I2C1->DR = (uint8_t)(SensorAddr << 1);						// Send slave address with write access
	
	while(!(I2C1->SR1 & I2C_SR1_ADDR));							// Wait for address sent flag
	
	DMA_Transmit(pWriteBuffer, NumByteToWrite);
	
	(void)I2C1->SR1;													// Read SR1
	(void)I2C1->SR2;													// Read SR2
}
static void DMA_Receive(const uint8_t *pBuffer, uint16_t size)
{
	if(NULL != pBuffer)
	{
		DMA1_Stream5->CR &= ~DMA_SxCR_EN;						// Disable Stream5
		while((DMA1_Stream5->CR) & DMA_SxCR_EN);
		
		/* Set Memory addresses and data items */
		DMA1_Stream5->M0AR = (uint32_t)pBuffer;
		DMA1_Stream5->PAR  = (uint32_t)&I2C1->DR;
		DMA1_Stream5->NDTR = size;
		
		/* Clear all Interrupt Flags */
		DMA1->HIFCR |= DMA_HIFCR_CTCIF5;
		
		DMA1_Stream5->CR |= DMA_SxCR_EN;							// Enable Stream5
	}
}
void I2C1_Read(uint8_t SensorAddr, uint8_t ReadAddr, uint8_t *pReadBuffer, uint16_t NumByteToRead)
{
	while(I2C1->SR2 & I2C_SR2_BUSY);								// Wait while Communication ongoing on the bus
	
	I2C1->CR1 |= I2C_CR1_START;									// generate start condition
	while(!(I2C1->SR1 & I2C_SR1_SB));							// Wait for start bit flag
	(void)I2C1->SR1;													// Read SR1
	
	I2C1->DR = (uint8_t)(SensorAddr << 1 | 0);				// Send slave address with write 
	while(!(I2C1->SR1 & I2C_SR1_ADDR));							// Wait for address sent flag
	(void)I2C1->SR1;													// Read SR1
	(void)I2C1->SR2;													// Read SR2
	
	while(!(I2C1->SR1 & I2C_SR1_TXE));							// Wait while DR not empty 
	
	if(2 <= NumByteToRead){
		I2C1->CR1 |= I2C_CR1_ACK;									// Acknowledge enable */ 
		I2C1->DR =  (ReadAddr);										// Send register address to read with increment */
	} else{
		I2C1->CR1 &= ~I2C_CR1_ACK;									// Acknowledge disable */
		I2C1->DR =  ReadAddr;										// Send register address to read (single) */		
	}
	
	while(!(I2C1->SR1 & I2C_SR1_BTF));							// Wait while Data byte transfer not done
	
	I2C1->CR1 |= I2C_CR1_START;									// generate ReSTART
	while(!(I2C1->SR1 & I2C_SR1_SB));							// Wait for start bit flag
	(void)I2C1->SR1;													// Read SR1
	
	I2C1->DR = (uint8_t)(SensorAddr << 1 | (uint8_t)0x01);// Send slave address with read */
	
	while(!(I2C1->SR1 & I2C_SR1_ADDR));							// Wait for address sent flag
	
	DMA_Receive(pReadBuffer, NumByteToRead);
	
	(void)I2C1->SR1;													// Read SR1
	(void)I2C1->SR2;													// Read SR2
}
