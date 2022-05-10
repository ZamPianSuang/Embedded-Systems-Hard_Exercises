; Constants defined in file stm32f4xx.constants.s

; This following is added to remove the compiler warning.
    AREA    __DEFINES_STM32F4_xx_DUMMY, CODE, READONLY

; Memory addresses of GPIO port A and RCC (reset and clock control) data
; structure. These addresses are predefined by the chip manufacturer.
GPIOA_BASE 		EQU		(0x40020000)		; Base memory address
RCC_BASE		EQU		(0x40023800)		; Byte offset of ODR from the base
	
; Byte offset of each variable in the GPIO_TypeDef structure
GPIO_MODER		EQU		(0x00)
GPIO_OTYPER		EQU		(0x04)
GPIO_RESERVED	EQU		(0x06)
GPIO_OSPEEDR	EQU		(0x08)
GPIO_PUPDR		EQU		(0x0C)
GPIO_IDR		EQU		(0x10)
GPIO_RESERVED1	EQU		(0x12)
GPIO_ODR		EQU		(0x14)
GPIO_RESERVED2	EQU		(0x16)
GPIO_BSRRL		EQU		(0x18)
GPIO_BSRRH		EQU		(0x1A)
GPIO_LCKR		EQU		(0x1C)
GPIO_AFR0		EQU		(0x20)		; AFR[0]
GPIO_AFR1		EQU		(0x24)		; AFR[1]
GPIO_AFRL		EQU		(0x20)
GPIO_AFRH		EQU 	(0x24)
	
; Byte offset of variable AHB1ENR in the RCC_TypeDef structure
RCC_AHB1ENR		EQU		(0x30)
	
END
	