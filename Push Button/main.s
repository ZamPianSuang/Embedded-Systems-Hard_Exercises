; LD2 is in PA5
; Push Button is in PC13
; Description: Toggle LED if button is pressed

		INCLUDE stm32f4xx_constants.s
			
		AREA	myCode, CODE, READONLY
		EXPORT	__main
		ENTRY
__main	PROC
		
		BL		Init_RCC
		BL		Init_PA5
		BL		Init_PC13
		
loop	BL		check_button	
		CMP		r0, #1
		BEQ		pressed
		B		loop
		
; Toggle LED
pressed	LDR		r3, =GPIOA_BASE
		LDR		r1, [r3, #GPIO_ODR]			; Read the output data register
		EOR		r1, r1, #(1<<5)				; Set bit 5
		STR		r1, [r3, #GPIO_ODR]			; Save to the output data register
		B		loop
		
stop	B		stop
		ENDP
			
Init_RCC PROC
		; Enable the clock to GPIO port A and port C
		; Load address of reset and clock control (RCC)
		LDR		r2, =RCC_BASE				; Pseduo instruction
		LDR		r1, [r2, #RCC_AHB1ENR]		; r1 = RCC->AHB1ENR
		ORR		r1, r1, #5					; Set bit 0 & 2 of AHB1ENR
		STR		r1, [r2, #RCC_AHB1ENR]		; GPIO port A & C clock enable
		BX		LR
		ENDP
			
Init_PA5 PROC
		; Load GPIO port A base address
		LDR		r3, =GPIOA_BASE
		
		; Set pin 5 I/O mode as general-purpose output
		LDR		r1, [r3, #GPIO_MODER]		; Read the mode register
		BIC		r1, r1, #(3<<10)			; clear bits 10 and 11
		ORR		r1, r1, #(1<<10)			; Set mode as digital output (mode = 01)
		STR		r1, [r3, #GPIO_MODER]		; Save to the mode register
		
		; Set pin 5 the push-pull mode for the output type
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
		
		BX		LR
		ENDP
			
Init_PC13 PROC
		; Load GPIO port C base address
		LDR		r3, =GPIOC_BASE
		
		; Set pin 13 I/O mode as general-purpose input
		LDR		r1, [r3, #GPIO_MODER]		; Read the mode register
		BIC		r1, r1, #(3<<26)			; clear bits 26 and 27
		STR		r1, [r3, #GPIO_MODER]		; Save to the mode register
		
		; Set I/O as no pull-up, no pull-down
		LDR		r1, [r3, #GPIO_PUPDR]		; r1 = GPIOB->PUPDR
		BIC		r1, r1, #(3<<26)			; No PUPD(00), PU(01), PD(10), Reserved(11)
		STR		r1, [r3, #GPIO_PUPDR]		; Save pull-up and pull-down setting
		
		BX		LR
		ENDP
			
; return in r0 (1 - pressed, 0 - not pressed)
check_button PROC
	PUSH	{LR}
		; Load GPIO port C base address
		LDR		r3, =GPIOC_BASE
		
		; Read Button Input		
read	LDR		r1, [r3, #GPIO_IDR]			; Read Input Data Register
		LSLS	r1, #18										
		LSRS	r0, r1, #31					; get IDR13 into r0
		
		CMP		r0, #1						; if (button not pressed) 
		MOVEQ	r0, #0						; return false
		BEQ		exit
		
		MOV		r2, #0						; counter = 0
		MOV		r4, #0						; loop i
		
check	CMP		r4, #100		
		BGE		exit
		
		; delay 
		BL		delay
		
		; Read Button Input		
		LDR		r1, [r3, #GPIO_IDR]			; Read Input Data Register
		LSLS	r1, #18										
		LSRS	r0, r1, #31					; get IDR13 into r0
		
		CMP		r0, #1						; if (button not pressed): 
		MOVEQ	r2, #0						; counter = 0
		ADDNE	r2, #1						; if(pressed): counter++
		
		CMP		r2, #100
		MOVGE	r0, #1						; if(counter >= 4):
		BGE		exit						; return true
		
		ADDS	r4, #1						; i++
		B		check
		
exit	POP {PC}
		ENDP

; delay about 5ms
delay	PROC
		PUSH 	{r4, LR}
		LDR		r4, =5000				; loop i
		
loop_d	CMP		r4, #0
		BLE		out
		SUBS	r4, #1
		B		loop_d

out		POP 	{r4, PC}
		ENDP
		;;;;;;;;;;;;
			
		ALIGN
		END
			