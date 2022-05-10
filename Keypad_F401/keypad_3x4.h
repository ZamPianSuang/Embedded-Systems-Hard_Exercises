// PC0 - PC2  <-- Input  Column
// PC8 - PC11 --> Output Row

#include "stm32f4xx.h"                  // Device header

void keypad_Init(void);
unsigned char findKey(unsigned int row, unsigned int col);
unsigned char keypad_scan(void);
void delayms(int delay);
