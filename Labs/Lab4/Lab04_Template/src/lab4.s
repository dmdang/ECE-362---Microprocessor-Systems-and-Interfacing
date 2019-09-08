.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

// GPIO addresses
.equ  GPIOC, 0x48000800
//.equ  PC2, 0x10
//.equ  PC0, 0x01
.equ PC2, 0x2
.equ PC0, 0x0
.equ  ODR, 0x14
.equ  AHBENR, 0x14
.equ  RCC, 0x40021000

// External interrupt for pins 0 and 1 is IRQ 5.
.equ EXTI0_1_IRQn,5

// SYSCFG constrol registers
.equ SYSCFG, 0x40010000
.equ EXTICR1, 0x8
.equ EXTICR2, 0xc
.equ EXTICR3, 0x10
.equ EXTICR4, 0x14

// External interrupt control registers
.equ EXTI, 0x40010400
.equ IMR, 0
.equ EMR, 0x4
.equ RTSR, 0x8
.equ FTSR, 0xc
.equ SWIER, 0x10
.equ PR, 0x14

// Variables to register things for pin 0
.equ EXTI_RTSR_TR0, 1
.equ EXTI_IMR_MR0, 1
.equ EXTI_PR_PR0, 1

// NVIC control registers...
.equ NVIC, 0xe000e000
.equ ISER, 0x100
.equ ICER, 0x180
.equ ISPR, 0x200
.equ ICPR, 0x280

// SysTick counter variables...
.equ SYST, 0xe000e000
.equ CSR, 0x10
.equ RVR, 0x14
.equ CVR, 0x18

//=======================================================
// Your translation of
// unsigned int fibonacci(unsigned int n) {
//   if (n < 2)
//      return n;
//   return fibonacci(n-1) + fibonacci(n-2);
// }
//
.global fibonacci
fibonacci:
	push {r4, r5, r6, r7, lr}
	cmp r0, #2
	blt end
	movs r4, r0
	subs r0, #1
	subs r5, r4, #2
	bl fibonacci
	cmp r5, #2
	bge righttree
	adds r0, r0, r5
	b end


righttree2:
	push {r4, r5, r6, r7, lr}
righttree:
	cmp r5, #2
	blt end
	movs r7, r5
	subs r5, #1
	subs r6, r7, #2
	bl righttree2
	cmp r5, #1
	beq r5one
	cmp r6, #1
	beq r6one
	cmp r6, #2
	bge movetofive

movetofive:
	movs r5, r6
	b righttree


r5one:
	adds r0, r0, r5
	b end
r6one:
	adds r0, r0, r6
	b end


end:
	pop {r4, r5, r6, r7, pc}
	bx lr // Student should remove this instruction.

//=======================================================
// Your implementation of a SysTick interrupt handler.
// This is an interrupt service routine.
// Increment the value of the global variable tick_counter
// Display that value with a call to display_digit().
//
.type SysTick_Handler, %function
.global SysTick_Handler
SysTick_Handler:
	push {lr}

	ldr r1, =tick_count
	ldr r0, [r1]
	adds r0, r0, #1
	str r0, [r1]
	//bl display_digit

    // Student code goes here.
    pop  {pc}

//=======================================================
// Initialize the SysTick counter.
// You should set the SYST_RVR (reset value register)
// so an exception occurs once per 100 milliseconds.
//
// Then set the SYST_CSR (control status register) so
// that it uses the CPU clock as the clock source, enable
// the SysTick exception request, and enable the counter.
//
.global init_systick
init_systick:
	push {lr}

	ldr r3, =SYST
	ldr r0, =4800000
	str r0, [r3, #RVR]

	movs r0, #7
	str r0, [r3, #CSR]

    pop {pc} // Student should remove this.

//=======================================================
// OR the value EXTI_RTSR_TR0 into the EXTI_RTSR
// (rising trigger selection register).
// This will tell the EXTI system to flag an interrupt
// on the rising edge of Pin 0.
//
.global init_rtsr
init_rtsr:

	ldr r0, =EXTI_RTSR_TR0
	ldr r1, =EXTI
	ldr r2, [r1, #RTSR]
	orrs r2, r2, r0
	str r2, [r1, #RTSR]

	bx lr // Student should remove this instruction.

//=======================================================
// OR the value EXTI_IMR_MR0 into EXTI_IMR
// (Interrupt mask register).
// This will unmask the external interrupt for Pin 0.
//
.global init_imr
init_imr:

	ldr r0, =EXTI_IMR_MR0
	ldr r1, =EXTI
	ldr r2, [r1, #IMR]
	orrs r2, r2, r0
	str r2, [r1, #IMR]

	bx lr // Student should remove this instruction.

//=======================================================
// Write (1 << EXTI0_1_IRQn) into the NVIC_ISER
// (Interrupt set enable register).
// (This value is '1' shifted left by EXTI0_1_IRQn bits.)
// This action will enable the external interrupt for pins 0 and 1.
//
.global init_iser
init_iser:

	push {r4, lr}

	movs r0, #1
	ldr r2, =NVIC
	ldr r3, =EXTI0_1_IRQn
	lsls r0, r0, r3
	ldr r3, =ISER
	str r0, [r2, r3]

	pop {r4, pc} // Student should remove this instruction.

//=======================================================
// The interrupt handler for Pins 0 and 1.
// The handler should increment the global variable named
// 'button_presses' and call display_digit with that value.
// Then it should write EXTI_PR_PR0 to the EXTI_PR register to
// clear the interrupt.
//
// Optionally, you may also call micro_wait() for a
// while to debounce the button press.
//
.type EXTI0_1_IRQHandler, %function
.global EXTI0_1_IRQHandler
EXTI0_1_IRQHandler:
    push {lr}

	//ldr r0, =button_presses
	//adds r0, r0, #1
	//bl display_digit

	ldr r1, =SYST
	ldr r2, [r1, #CSR]
	movs r3, #0x1
	lsls r3, #2
	ands r2, r2, r3
	str r2, [r1, #CSR]


	ldr r1, =EXTI_PR_PR0
	ldr r3, =EXTI
	ldr r0, [r3, #PR]
	orrs r0, r0, r1
	str r0, [r3, #PR]

    // Student code goes here.
    pop {pc}

//=======================================================
// "bit bang" the value of data bit by bit from LSB to
// MSB into PC2, while also generating a bit banged serial
// clock. Bit banging is embedded speak for toggling the
// output depending on the current bit. As an example
// consider 0b1001 0010, bit banging this value will set
// PC2 output to the following values logic 0, logic 1,
// logic 0, logic 0, logic 1, logic 0, logic 0, logic 1.
// This is bit banging from LSB to MSB.

.global setpin
setpin:
    push    {r2, r3, lr}
    /* Student code goes here */

	ldr r3, [r0, #ODR]
	movs r2, #0x1
	lsls r2, r1
	orrs r3, r3, r2
	str r3, [r0, #ODR]

    /* End of student code*/
    pop     {r2, r3, pc}

.global clrpin
clrpin:
    push    {r2, r3, lr}
    /* Student code goes here */

	ldr r3, [r0, #ODR]
    movs r2, #0x1
    lsls r2, r1
    mvns r2, r2
    ands r3, r3, r2
	str r3, [r0, #ODR]

    /* End of student code*/
    pop     {r2, r3, pc}

.global send_data
send_data:

	push {r4, r5, lr}

	movs r1, #16
	movs r2, #0
	// r0 is data

for_loop:

	movs r1, #16
	cmp r2, r1
	bge exit
	movs r3, #1
	movs r4, r0 // data inside r4
	movs r5, r0 // data inside r5 (r0 is now r5)
	ldr r0, =GPIOC // load r0 with GPIOC
	ldr r1, =PC2 // load r1 with PC2
	ands r3, r3, r4
	cmp r3, #1
	beq set
	b clr

set:
	bl setpin
	b bitbang

clr:
	bl clrpin
	b bitbang



bitbang:
	ldr r0, =GPIOC
	ldr r1, =PC0
	bl setpin

	movs r3, r0 // GPIOC into r3
	movs r0, #200
	bl nano_wait

	movs r0, r3 // GPIOC back into r0, PC0 still in r1
	bl clrpin

	movs r3, r0 // GPIOC into r3
	movs r0, #200
	bl nano_wait

	movs r0, r5 // put data back into r0
	lsrs r0, r0, #0x1

	adds r2, r2, #1
	b for_loop

exit:
	pop {r4, r5, pc} // Student should remove this instruction.
