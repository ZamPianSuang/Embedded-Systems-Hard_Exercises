// USART1_Tx - PA9, PB6  & USART1_Rx - PA10, PB7
// UART4_Tx  - PA0, PC10 & UART4_Rx  - PA1, PC11

#include "USARTx.h"

static unsigned char buffer[13] = "Hello USART2 ";

int main(void)
{
	USART2_Rx_Init();
	
	while(1)
	{
		USARTx_Read(USART2, buffer, 13);
	}
}
