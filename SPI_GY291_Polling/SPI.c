#include "SPI.h"

void SPI1_Init(void)
{
	/* SPI1 - APB2 (90MHz)
		SPI1_NSS/CS	 	- PA4 - AF5	
	   SPI1_SCK  		- PA5 - AF5
		SPI1_MISO 		- PA6 - AF5
	   SPI1_MOSI 		- PA7 - AF5 */
	
	/********** Enable Clocks **********/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;					// Enable GPIOA clock
	RCC->APB2ENR  |= RCC_APB2ENR_SPI1EN;					// Enable SPI1 clock
	RCC->APB2RSTR |= RCC_APB2RSTR_SPI1RST;					// Reset SPI1
	RCC->APB2RSTR &= ~RCC_APB2RSTR_SPI1RST;				// Clear the reset of SPI1
	
	/********** GPIO Configurations **********/
	GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
	// PA4 - Ouput mode; PA5, PA6, PA7 - Alt function mode										
	GPIOA->MODER |= (GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);		
	
	GPIOA->AFR[0] |= (GPIO_AFRL_AFRL5_0 | GPIO_AFRL_AFRL5_2);			// PA5 - AF5
	GPIOA->AFR[0] |= (GPIO_AFRL_AFRL6_0 | GPIO_AFRL_AFRL6_2);			// PA6 - AF5
	GPIOA->AFR[0] |= (GPIO_AFRL_AFRL7_0 | GPIO_AFRL_AFRL7_2);			// PA7 - AF5
	
	/********** SPI1 Configurations **********/
	SPI1->CR1 &= ~SPI_CR1_SPE;									// Disable SPI
	SPI1->CR1 &= ~SPI_CR1_RXONLY;								// Full duplex (Transmit and Receive)
	SPI1->CR1 &= ~SPI_CR1_BIDIMODE;							// 2-line unidirectional data mode
	SPI1->CR1 &= ~SPI_CR1_BIDIOE;								// Output disabled (receive-only mode)
	SPI1->CR1 &= ~(SPI_CR1_DFF | SPI_CR1_LSBFIRST);		// 8-bit Data Frame, MSB first
	SPI1->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);			// CPOL = 1, CPHA = 1
	SPI1->CR1 |=  SPI_CR1_BR_0;								// 001 Fclk/4 = 4 MHz (@16 MHz)
	SPI1->CR1 &= ~SPI_CR1_CRCEN;								// Disable Hardware CRC calculation
	SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_SSI);				// Enable software slave management
	SPI1->CR1 |=  SPI_CR1_MSTR;								// Master mode
	SPI1->CR2 &= ~SPI_CR2_FRF;									// SPI Motorola mode
	SPI1->CR2 |= SPI_CR2_SSOE;									// Disable MultiMaster Mode
	SPI1->CR1 |=  SPI_CR1_SPE;									// Enable SPI
}
void SPI1_Write(uint8_t *txBuffer, uint8_t *rxBuffer, int size)
{
	int i = 0;
	for(i = 0; i < size; i++)
	{
		while(!(SPI1->SR & SPI_SR_TXE));						// Wait for TXE (Tx buffer Empty)
		SPI1->DR = txBuffer[i];
		while(!(SPI1->SR & SPI_SR_RXNE));					// Wait for RXE (Rx buffer Not Empty)
		rxBuffer[i] = (uint8_t)SPI1->DR;
	}
	while(SPI1->SR & SPI_SR_BSY);								// Wait for BSY flag cleared
}
void SPI1_Read(uint8_t *rxBuffer, int size)
{
	int i = 0;
	for(i = 0; i < size; i++)
	{
		while(!(SPI1->SR & SPI_SR_TXE));						// Wait for TXE (Tx buffer Empty)
		SPI1->DR = 0xFF;											// Master must send dummy byte to read from Slave
		while(!(SPI1->SR & SPI_SR_RXNE));					// Wait for RXE (Rx buffer Not Empty)
		rxBuffer[i] = (uint8_t)SPI1->DR;
	}
	while(SPI1->SR & SPI_SR_BSY);								// Wait for BSY flag cleared
}
void GYRO_IO_Write(uint8_t *pBuffer, uint8_t WriteAddr, uint8_t size)
{
	uint8_t rxBuffer[32];
	if(size > 1)
		WriteAddr |= 1U << 6;									// Select the ode of writing multiple-byte
	
	/* Set SPI interface */
	ADXL345_CS_LOW;												// 0 = SPI, 1 = I2C
	delayMs(10);													// Short delay
	
	/* Send the address of the indexed register */
	SPI1_Write(&WriteAddr, rxBuffer, 1);
	
	/* Send the data that will be written into the device */
	SPI1_Write(pBuffer, rxBuffer, size);
	
	/* Set Chip Select High at the end of transmission */
	delayMs(10);
	ADXL345_CS_HIGH;												// 0 = SPI, 1 = I2C
}
void GYRO_IO_Read(uint8_t *pBuffer, uint8_t ReadAddr, uint8_t size)
{
	uint8_t rxBuffer[32];
	// Select read & multiple-byte mode
	uint8_t AddrByte = ReadAddr | 1U << 7 | 1U << 6;
	
	// Set chip select low at the start of the transmission
	ADXL345_CS_LOW;
	delayMs(10);
	
	// Send the address of the indexed register
	SPI1_Write(&AddrByte, rxBuffer, 1);
	
	// Receive the data that will be read from the device (MSB first)
	SPI1_Read(pBuffer, size);
	
	// Set chip select High at the end of the transmission
	delayMs(10);
	ADXL345_CS_HIGH;
}
void delayMs(int delay)
{
	SysTick->LOAD = 16000-1; 									// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;					
	SysTick->CTRL |= 0x05;				
										
	for (int i = 0; i < delay; i++)	 
	{
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	SysTick->CTRL = 0;	
}
