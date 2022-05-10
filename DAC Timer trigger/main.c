#include "DAC.h"

static volatile uint32_t i, output = 0;

int main(void)
{
	//TIM4_Init();
	DAC_channel1_Init();
	
	while(1)
	{
		// Set DAC output
		DAC->DHR12R1 = output;							// Channel 1 12-bit right-aligned data
		// Hardware clears SWTRIGR once DHR12R1 has been copied to DOR
		DAC->SWTRIGR |= DAC_SWTRIGR_SWTRIG1;		// Start software trigger
		for(i = 0; i <= 10; i++);						// Short software delay
		output = (output + 1) & 0xFF;				// Increment output voltage
	}
}
