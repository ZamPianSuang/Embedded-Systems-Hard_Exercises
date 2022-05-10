// C1 - C3 => PA0  - PA2  -- Input Column Ports (external pull-up)
// R1 - R4 => PB12 - PB15 -- Output Row Ports

#include "stm32f4xx.h"                  // Device header

static unsigned char key_map[4][3] = 
{
	{'1','2','3'},
	{'4','5','6'},
	{'7','8','9'},
	{'*','0','#'}
};

void keypad_Init(void);
unsigned char findKey(unsigned int row, unsigned int col);
unsigned char keypad_scan(void);
void DelayMs(int delay);
