; LD2 is in PA5

		INCLUDE stm32f4xx_constants.s
			
		AREA	myCode, CODE, READONLY
		EXPORT	__main
		ENTRY
__main	PROC
		; Enable the clock to GPIO port B
		; Load address of reset and clock control (RCC)
		LDR		r2, =RCC_BASE				; Pseduo instruction
		LDR		r1, [r2, #RCC_AHB1ENR]		; r1 = RCC->AHB1ENR
		ORR		r1, r1, #1					; Set bit 1 of AHB1ENR
		STR		r1, [r2, #RCC_AHB1ENR]		; GPIO port B clock enable
		
		; Load GPIO port A base address
		LDR		r3, =GPIOA_BASE
		
		BL		Init_PA5
		
		; Light up LED
		LDR		r1, [r3, #GPIO_ODR]			; Read the output data register
		ORR		r1, r1, #(1<<5)				; Set bit 2
		STR		r1, [r3, #GPIO_ODR]			; Save to the output data register
		
stop	B		stop
		ENDP
			
Init_PA5 PROC
	
		; Set pin 2 I/O mode as general-purpose output
		LDR		r1, [r3, #GPIO_MODER]		; Read the mode register
		BIC		r1, r1, #(3<<10)			; Direction mask pin 6, clear bits 10 and 11
		ORR		r1, r1, #(1<<10)			; Set mode as digital output (mode = 01)
		STR		r1, [r3, #GPIO_MODER]		; Save to the mode register
		
		; Set pin 2 the push-pull mode for the output type
		LDR		r1, [r3, #GPIO_OTYPER]		; Read the output type register
		BIC		r1, r1, #(1<<5)				; PA5 - Push-pull(0), open-drain(1)
		STR		r1, [r3, #GPIO_OTYPER]		; Save to the output type register
		
		; Set I/O output speed value as low
		LDR		r1, [r3, #GPIO_OSPEEDR]		; Read the output speed register
		BIC		r1, r1, #(3<<4)				; Low(00), Medium(01), Fast(01), High(11)
		STR		r1, [r3, #GPIO_OSPEEDR]		; Save to the output speed register
		
		; Set I/O as no pull-up, no pull-down
		LDR		r1, [r3, #GPIO_PUPDR]		; r1 = GPIOB->PUPDR
		BIC		r1, r1, #(3<<10)			; No PUPD(00), PU(01), PD(10), Reserved(11)
		STR		r1, [r3, #GPIO_PUPDR]		; Save pull-up and pull-down setting
		
		ENDP
		ALIGN
		END
			