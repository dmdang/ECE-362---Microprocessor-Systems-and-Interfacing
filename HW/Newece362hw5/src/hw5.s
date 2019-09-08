.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.equ RCC,       0x40021000
.equ GPIOA,     0x48000000
.equ GPIOC,     0x48000800
.equ AHBENR,    0x14
.equ IOPAEN,    0x20000
.equ IOPCEN,    0x80000
.equ MODER,     0x00
.equ OSPEEDR,   0x08
.equ PUPDR,     0x0c
.equ IDR,       0x10
.equ ODR,       0x14
.equ BSRR,      0x18
.equ BRR,       0x28
.equ PC8,       0x100

// SYSCFG control registers
.equ SYSCFG,    0x40010000
.equ EXTICR1,   0x08

// NVIC control registers
.equ NVIC,      0xe000e000
.equ ISER,      0x100

// External interrupt control registers
.equ EXTI,      0x40010400
.equ IMR,       0
.equ RTSR,      0x8
.equ PR,        0x14

// External interrupt for pins 2 and 3 is IRQ 6.
.equ EXTI2_3_IRQn,  6

//=====================================================================
// Q1
//=====================================================================
.global recmult
recmult:

	push {r4, lr}

	cmp r1, #0
	blt elseif
	cmp r1, #0
	beq return0
	movs r4, r0 // save original x
	subs r1, r1, #1
	bl recmult
	adds r0, r0, r4
	b end2

elseif:
	movs r4, r0
	adds r1, r1, #1
	bl recmult
	subs r0, r0, r4
	b end2


return0:
	movs r0, #0
	b end2

end2:
	pop {r4, pc}

//=====================================================================
// Q2
//=====================================================================
.global enable_porta
enable_porta:

	push {lr}

	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	movs r2, #0x1
	lsls r2, r2, #17
	orrs r1, r2, r1
	str r1, [r0, #AHBENR]

	pop {pc}

//=====================================================================
// Q3
//=====================================================================
.global enable_portc
enable_portc:

	push {lr}

	ldr r0, =RCC
	ldr r1, [r0, #AHBENR]
	movs r2, #0x1
	lsls r2, r2, #19
	orrs r1, r1, r2
	str r1, [r0, #AHBENR]

	pop {pc}

//=====================================================================
// Q4
//=====================================================================
.global setup_pa1
setup_pa1:

	push {lr}

	// set pa1 as input
	ldr r0, =GPIOA
	ldr r1, [r0, #MODER]
	movs r2, #3
	lsls r2, r2, #2
	mvns r2, r2
	ands r1, r1, r2
	str r1, [r0, #MODER]

	// clear PUPDR
	ldr r1, [r0, #PUPDR]
	movs r2, #3
	lsls r2, r2, #2
	mvns r2, r2
	ands r1, r1, r2

	// set pull-up resistor
	movs r2, #1
	lsls r2, r2, #2
	orrs r1, r1, r2
	str r1, [r0, #PUPDR]

	pop {pc}

//=====================================================================
// Q5
//=====================================================================
.global setup_pa2
setup_pa2:

	push {lr}

	// set pa2 as input
	ldr r0, =GPIOA
	ldr r1, [r0, #MODER]
	movs r2, #3
	lsls r2, r2, #4
	mvns r2, r2
	ands r1, r1, r2
	str r1, [r0, #MODER]

	// clear pin2 in PUPDR
	ldr r1, [r0, #PUPDR]
	movs r2, #3
	lsls r2, r2, #4
	mvns r2, r2
	ands r1, r1, r2

	//set pull-down resistor
	movs r2, #2
	lsls r2, r2, #4
	orrs r1, r1, r2
	str r1, [r0, #PUPDR]


	pop {pc}

//=====================================================================
// Q6
//=====================================================================
.global setup_pc8
setup_pc8:

	push {lr}

	// setup pc8 as output
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]

	// clear pin8 of moder
	movs r2, #3
	lsls r2, r2, #16
	mvns r2, r2
	ands r1, r1, r2

	// set pin8 as output
	movs r2, #1
	lsls r2, r2, #16
	orrs r1, r1, r2
	str r1, [r0, #MODER]

	// clear pin8 of ospeedr
	ldr r1, [r0, #OSPEEDR]
	movs r2, #3
	lsls r2, r2, #16
	mvns r2, r2
	ands r1, r1, r2

	// set output speed high
	movs r2, #3
	lsls r2, r2, #16
	orrs r1, r1, r2
	str r1, [r0, #OSPEEDR]

	pop {pc}

//=====================================================================
// Q7
//=====================================================================
.global setup_pc9
setup_pc9:

	push {lr}

	// set pc9 as output
	ldr r0, =GPIOC
	ldr r1, [r0, #MODER]

	// clear value of pin9 in moder
	movs r2, #3
	lsls r2, r2, #18
	mvns r2, r2
	ands r1, r1, r2

	// set pc9 as output
	movs r2, #1
	lsls r2, r2, #18
	orrs r1, r1, r2
	str r1, [r0, #MODER]

	// clear value of pin9 in ospeedr
	ldr r1, [r0, #OSPEEDR]
	movs r2, #3
	lsls r2, r2, #18
	mvns r2, r2
	ands r1, r1, r2

	// set pc9 output speed as medium
	movs r2, #1
	lsls r2, r2, #18
	orrs r1, r1, r2
	str r1, [r0, #OSPEEDR]

	pop {pc}

//=====================================================================
// Q8
//=====================================================================
.global action8
action8:

	push {lr}

	// read pa1 and pa2 (input modes)
	//check pa1
	ldr r0, =GPIOA
	ldr r1, [r0, #IDR]
	movs r2, #1
	lsls r2, #1
	ands r2, r2, r1 // r2 is 1 if pa1 is HIGH
	cmp r2, #2
	beq checkpa2
	b else

checkpa2:
	//check pa2
	lsls r2, #1
	ands r2, r2, r1 // r2 is 0 if pa2 is LOW
	cmp r2, #0
	beq setpc8h
	b else

setpc8h:

	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	movs r2, #1
	lsls r2, r2, #8
	orrs r1, r1, r2
	str r1, [r0, #ODR]
	b end

else:

	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	movs r2, #1
	lsls r2, r2, #8
	mvns r2, r2
	ands r1, r1, r2
	str r1, [r0, #ODR]
	b end

end:
	pop {pc}

//=====================================================================
// Q9
//=====================================================================
.global action9
action9:

	push {lr}

	// read pa1 and pa2 (input modes)
	//check pa1
	ldr r0, =GPIOA
	ldr r1, [r0, #IDR]
	movs r2, #1
	lsls r2, #1
	ands r2, r2, r1 // r2 is 0 if pa1 is LOW
	cmp r2, #0
	beq checkpa21
	b else1

checkpa21:
	//check pa2
	movs r2, #1
	lsls r2, #2
	ands r2, r2, r1 // r2 is 4 if pa2 is HIGH
	cmp r2, #4
	beq setpc9h
	b else1

setpc9h:

	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	movs r2, #1
	lsls r2, r2, #9
	orrs r1, r1, r2
	str r1, [r0, #ODR]
	b end1

else1:

	ldr r0, =GPIOC
	ldr r1, [r0, #ODR]
	movs r2, #1
	lsls r2, r2, #9
	mvns r2, r2
	ands r1, r1, r2
	str r1, [r0, #ODR]
	b end1

end1:
	pop {pc}


//=====================================================================
// Q10
//=====================================================================
.type EXTI2_3_IRQHandler, %function
.global EXTI2_3_IRQHandler
EXTI2_3_IRQHandler:

	push {lr}

	// write 1 to bit 2 of EXTI_PR reg
	ldr r0, =EXTI
	ldr r1, [r0, #PR]
	movs r2, #1
	lsls r2, r2, #2
	orrs r1, r1, r2
	str r1, [r0, #PR]

	// increment global var counter
	ldr r0, =counter
	ldr r1, [r0]
	adds r1, r1, #1
	str r1, [r0]


	pop {pc}

//=====================================================================
// Q11
//=====================================================================
.global enable_exti2
enable_exti2:

	push {lr}

	// setup SYSCFG_EXTICR1 to use pa2 for interrupt source
	// put 0000 in EXTI2
	ldr r0, =SYSCFG
	ldr r1, [r0, #EXTICR1]
	movs r2, #0xf
	lsls r2, r2, #8
	mvns r2, r2
	ands r1, r1, r2
	str r1, [r0, #EXTICR1]

	// configure EXTI_RTSR to trigger on rising edge of PA2
	ldr r0, =EXTI
	ldr r1, [r0, #RTSR]
	movs r2, #1
	lsls r2, r2, #2
	orrs r1, r1, r2
	str r1, [r0, #RTSR]

	// set EXTI_IMR to not ignore pin number 2
	ldr r1, [r0, #IMR]
	movs r2, #1
	lsls r2, r2, #2
	orrs r1, r1, r2
	str r1, [r0, #IMR]

	// configure NVIC to enable EXTI2_3_IRQHandler
	ldr r0, =NVIC
	ldr r1, =EXTI2_3_IRQn
	movs r2, #1
	lsls r2, r2, r1
	ldr r3, =ISER
	str r2, [r0, r3]


	pop {pc}
