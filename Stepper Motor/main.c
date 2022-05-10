/*
	Unipolar
	Rated voltage : 5VDC 
	Number of Phase : 4 
	Speed Variation Ratio : 1/64 (inner rotor need 64 turns for outer rotor 1 turn)
	Stride Angle : 5.625° /64 
	Frequency : 100Hz (10ms)
	---
	Step Angle: 11.25 degree (for Full and Wave steppping)
	Step Angle: 5.625 degree (for Half stepping)
	4 Layers of 8 Teeth
	One set of Teeth per PHASE
	32 Teeth overall
	One Tooth per STEP
*/

#include "stm32f4xx.h"                  // Device header

void GPIO_Init(void);
void Circle_ClockWise_Full(void);
void Circle_Counter_Half(void);
void delayMs(int delay);

// 4.Orange, 3.Yellow, 2.Pink, 1.Blue	----  0001, 0011, 0010, 0110, 0100, 1100, 1000, 1001
static unsigned char Halfstep[8] 			= {0x01, 0x03, 0x02, 0x06, 0x04, 0x0C, 0x08, 0x09}; 	
static unsigned char Fullstep[4] 			= {0x03, 0x06, 0x0C, 0x09}; 
// 4.Orange, 3.Yellow, 2.Pink, 1.Blue	----  0011, 0110, 1100, 1001

int main(void)
{
	GPIO_Init();
	
	Circle_Counter_Half();
	delayMs(1000);
	Circle_ClockWise_Full();
	
	while(1)
	{
		
	}
}
void GPIO_Init(void)
{
	/** PC0-IN1, PC1_IN2, PC2-IN3, PC3-IN4 **/
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
	// 01: General purpose output mode
	GPIOC->MODER &= ~(0xFFU);
	GPIOC->MODER |= 0x55;
	// 0: Output push-pull (reset state)
	GPIOC->OTYPER &= ~(0xFU);
	// 10: Fast speed
	GPIOC->OSPEEDR |= 0xAA;
	// 00: No pull-up, pull-down
	GPIOC->PUPDR &= ~(0xFFU);
}
void Circle_ClockWise_Full(void)
{
	// 11.25 degree * 32 = 360 degree
	// 32 * 64 = 2048 (512 * 4 = 2048)
	for(int i = 0; i < 512; i++)
	{
		for(int j = 0; j < 4; j++) // 1 phase
		{
			GPIOC->ODR = Fullstep[j];
			delayMs(2);
		}
	}
}
void Circle_Counter_Half(void)
{
	// 5.625 degree * 64 = 360 degree
	// 64 * 64 = 4096 (512 * 8 = 4096)
	for(int i = 0; i < 512; i++)
	{
		for(int j = 7; j >= 0; j--) // 1 phase
		{
			GPIOC->ODR = Halfstep[j];
			delayMs(1);
		}
	}
}

void delayMs(int delay)	// see User Guide Note: LOAD is automatically reload again n again
{
	SysTick->LOAD = 16000; 		// 16 Mhz means 16000 clock per milisecond
	SysTick->VAL = 0;				// Clear Current Value Register
	SysTick->CTRL |= 0x05;		// 101 - 1:processor clock, 1:counter enabled
		// 0:counting down to zero does not assert the SysTick exception request
	
	for (int i = 0; i < delay; i++)//[16]Returns 1 if timer counted to 0 since last time this was read. 
	{
		// Wait until the COUNT flag is set
		while ((SysTick->CTRL & 0x10000) == 0);
	}
}
