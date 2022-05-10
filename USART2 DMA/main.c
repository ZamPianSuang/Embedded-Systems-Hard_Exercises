#include "USARTx_DMA_Rx.h"

static uint8_t value = 0;

int main(void)
{
	USART2_GPIO_Rx_Init();
	
	DMA_Init();
	DMA_Config((uint32_t)&USART2->DR, (uint32_t)&value, 1);
	
	while(1)
	{
		
	}
}
