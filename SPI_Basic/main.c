// Implementing SK9822 RGB LED

#include "stm32f4xx.h"                  // Device header
#define LED_COUNT 30
#define GLOBAL 	0
#define RED			3
#define GREEN		2
#define BLUE		1

static uint8_t LED_ARRAY[LED_COUNT][4];

void SPI_Init(void);
void SPI_Send(uint8_t);
void clearArray(void);
void skSetLed(uint8_t index, uint8_t global, uint8_t red, uint8_t green, uint8_t blue);
void skUpdate(void);

int main(void)
{
	SPI_Init();
	clearArray();
	
	skSetLed(10, 31, 0, 0, 255);
	skUpdate();
	
	uint8_t i;
	
	while(1)
	{
		for(i = 0; i < LED_COUNT; i++)
		{
			skSetLed(i, 31, 0, 0, 255);
			skUpdate();
			skSetLed(i, 0, 0, 0, 0);
			skUpdate();
		}
	}
}

void SPI_Init(void)
{
	/* SPI1 - APB2 (90MHz)
	   SPI1_SCK  - PA5 - AF5
	   SPI1_MOSI - PA7 - AF5 */
	
	/********** Enable Clocks **********/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
	
	/********** GPIO Configurations **********/
	GPIOA->MODER &= ~(GPIO_MODER_MODE5 | GPIO_MODER_MODE7);
	GPIOA->MODER |= (GPIO_MODER_MODE5_1 | GPIO_MODER_MODE7_1);		// Alternate function
	
	GPIOA->AFR[0] |= (GPIO_AFRL_AFRL5_0 | GPIO_AFRL_AFRL5_2);		// PA5 - AF5
	GPIOA->AFR[0] |= (GPIO_AFRL_AFRL7_0 | GPIO_AFRL_AFRL7_2);		// PA7 - AF5
	
	/********** SPI1 Configurations **********/
	SPI1->CR1 |= (SPI_CR1_BIDIMODE | SPI_CR1_BIDIOE);					// 1-line direction| Transmit only
	SPI1->CR1 &= ~(SPI_CR1_DFF | SPI_CR1_LSBFIRST);						// 8-bit data frame format| MSB first
	SPI1->CR1 |= (SPI_CR1_SSM | SPI_CR1_BR_1 | SPI_CR1_MSTR);		// Enable SSM| Fclk/8 = 2 Mhz| Master
	SPI1->CR1 |= (SPI_CR1_CPOL | SPI_CR1_CPHA);							// CPOL = 1| CPHA = 1 (dev's datasheet)
	SPI1->CR2 |= SPI_CR2_SSOE;								// Disable MultiMaster Mode

	SPI1->CR1 |= SPI_CR1_SPE;								// Enable SPI 
}
void SPI_Send(uint8_t data)
{
	SPI1->DR = data;
	while(!(SPI1->SR & SPI_SR_TXE));						// Wait while Tx buffer is not empty
}
void clearArray(void)
{
	int i, j;
	for(i = 0; i < LED_COUNT; i++)
	{
		LED_ARRAY[i][GLOBAL] = 0xE0;						// 0 brightness
		for(j = 1; j < 4; j++)
		{
			LED_ARRAY[i][j] = 0x00;							// 0 RBG values
		}
	}
}
void skSetLed(uint8_t index, uint8_t global, uint8_t red, uint8_t green, uint8_t blue)
{
	LED_ARRAY[index][GLOBAL] = (0xE0 | global);		// global variable should not be more than 31
	LED_ARRAY[index][RED]   = red;
	LED_ARRAY[index][GREEN] = green;
	LED_ARRAY[index][BLUE]  = blue;
}
void skUpdate(void)
{
	uint8_t i, j;
	// Send Start Frame
	for(i = 0; i < 4; i++)
	{
		SPI_Send(0x00);
	}
	// Send LED Frame
	for(i = 0; i < LED_COUNT; i++)
	{
		for(j = 0; j < 4; j++)
		{
			SPI_Send(LED_ARRAY[i][j]);
		}
	}
	// Send Stop Frame
	for(i = 0; i < 4; i++)
	{
		SPI_Send(0xFF);
	}
}
