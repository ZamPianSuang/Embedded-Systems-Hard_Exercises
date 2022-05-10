// I2C1_SCL - PB8 - AF4
// I2C1_SDA - PB9 - AF4

#include "wire.h"

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
void I2C_readbyte(char saddr, char maddr, uint8_t *data)
{
	uint32_t temp;
	I2C1->CR1 |= I2C_CR1_STOP;										// Stop generation
	
	while(I2C1->SR2 & I2C_SR2_BUSY);								// Wait while Communication ongoing on the bus
	
	I2C1->CR1 |= I2C_CR1_START;									// generate start condition
	while(!(I2C1->SR1 & I2C_SR1_SB));							// Wait for start bit flag
	
	I2C1->DR = (uint8_t)(saddr << 1);
	while(!(I2C1->SR1 & I2C_SR1_ADDR));							// Wait for address sent flag
	temp = I2C1->SR2;													// Procedure to clear ADDR flag
	
	while(!(I2C1->SR1 & I2C_SR1_TXE));							// Wait while DR not empty 
	I2C1->DR = maddr;
	while(!(I2C1->SR1 & I2C_SR1_TXE));							// Wait while DR not empty 
	
	I2C1->CR1 |= I2C_CR1_START;									// generate start condition
	while(!(I2C1->SR1 & I2C_SR1_SB));							// Wait for start bit flag
	
	I2C1->DR = (uint8_t)(saddr << 1 | 1);
	while(!(I2C1->SR1 & I2C_SR1_ADDR));							// Wait for address sent flag
	I2C1->CR1 &= ~I2C_CR1_ACK;
	temp = I2C1->SR2;													// Procedure to clear ADDR flag
	
	I2C1->CR1 |= I2C_CR1_STOP;										// Stop generation
	while(!(I2C1->SR1 & I2C_SR1_RXNE));							// Wait while DR is empty
	
	*data++ = (uint8_t)I2C1->DR;
}
void delayMs(uint32_t delay)
{
	// Any value between 0x00000001 - 0x00FFFFFF (2^24 - 1)
	SysTick->LOAD = 16000-1; 				// 16 Mhz means 16,000 clock per Milisecond
	SysTick->VAL = 0;							// Clear Current Value Register
	SysTick->CTRL |= 0x05;					// 101 - 1:processor clock, 1:counter enabled
	
	for (uint32_t i = 0; i < delay; i++)		//[16]Returns 1 if timer counted to 0 since last time this was read. 
	{
		// Wait until the COUNT flag is set
		while ((SysTick->CTRL & 0x10000) == 0);
	}
	
	SysTick->CTRL = 0;						// stop the timer
}
