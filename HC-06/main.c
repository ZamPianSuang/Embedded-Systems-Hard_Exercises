#include "UART4_Tx_Rx_Dma_Int.h"

int main(void)
{
	PC10_PC11_UART4_Init();
	
	DMA_Init();
	DMA_Config((uint32_t)&UART4->DR, (uint32_t)&buffer, 1);
	
	while(1)
	{
		GPIOA->ODR |= 1 << 5;
	}
}
