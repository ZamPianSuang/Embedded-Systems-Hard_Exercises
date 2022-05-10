#include "USARTx_DMA_Rx_Int.h"

int main(void)
{
	USART2_GPIO_Rx_Init();
	
	DMA_Init();
	DMA_Config((uint32_t)&USART2->DR, (uint32_t)&buffer, 1);
	
	while(1)
	{

	}
}
