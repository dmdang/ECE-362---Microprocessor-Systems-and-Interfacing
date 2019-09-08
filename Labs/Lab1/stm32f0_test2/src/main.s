.syntax unified
.cpu cortex-m0
.fpu softvfp
.thumb

/******************************************************************************
* ECE 362 Lab expirement 1
* "First Blinks" with assembly
******************************************************************************/

/* Student code goes here */
/* Put the .equ here */
.equ	RCC,		0x40021000
.equ	AHBENR,		0x14
.equ	GPIOCEN,	0x00080000
.equ	GPIOC,		0x48000800
.equ	MODER,		0x00
.equ	ODR,		0x14
.equ	PIN8MOD,	0x00010000
.equ	PIN8,		0x00000100

/* End of student code */

.global main
main:

/****************************************************************************
* Code Segment 1
* Arithmetic operations.
*
* Description:
*   Write an assembly program to set the following register values using the
*   MOVS instruction.
*		R0 = 0x11
*		R1 = 0x33
*   Set R2 to be the sum of R0 and R1.
*   Set R3 to be the difference of R0 and R1.
*   Set R4 to be the product of R2 and R3.
*
* Useful Instructions:
*   Look at the ADD, SUB, and MUL instructions (it is up
*   to you to look up the mnemonics for these instructions)
*
****************************************************************************/
codeSegment1:
	/* Student code goes here */

	movs r0, #0x11
	movs r1, #0x33
	adds r2, r0, r1
	subs r3, r1, r0
	movs r4, r2
	muls r4, r3

	/* End of student code */

/****************************************************************************
* Code Segment 2
*
* Description:
*   Initialize r0 to 0xFFFFFFFE (a lower)
*   Initialize r1 to 0x7FFFFFFF (a upper)
*   Initialize r2 to 0x2        (b lower)
*   Initialize r3 to 0x0        (b upper)
*   When you write this code, do no use "LDR R0,=0xFFFFFFFE" to load R0.
*   Hint: for initializing r2 and r3, use the "movs" instruction.
*   Hint2: Initialize one of r0 or r1 then initialize the other
*
* Useful Instructions:
*   add, subtract, move, logical shift right, and logical shift left
*   (it is up to you to look up the mnemonics for these instructions)
*
****************************************************************************/
codeSegment2:
    /* Student code goes here*/

	movs r2, #0x2
	movs r3, #0x0
	subs r0, r2, #4
	lsrs r1, r0, #1

    /* End of student code*/

/****************************************************************************
* Code Segment 3
*
* Add two 64 bit numbers:
*
* Description:
*   Write a program to add 2 64-bit numbers.
*   The operation is a = a + b, where each a and b are 64 bits.
*   Given that
*     a's lower 32 bits are stored in r0,
*     a's upper 32 bits are stored in r1,
*     b's lower 32 bits are stored in r2,
*     b's upper 32 bits are stored in r3,
*   use add with carry instruction to propagate the
*   carry from the lower 32 bit's (lower word) to the upper word
*
* Useful Instructions:
*   add and add-with-carry (it is up to you to look up the
*   mnemonics for these instructions)
*   Think: ask yourself why you need the <s> suffix
*
****************************************************************************/
codeSegment3:
    /* Student code goes here*/

	adds r0, r2
	adcs r1, r3

    /* End of student code */


/****************************************************************************
* Code Segment 4
*
* Blink the LED on PC8.
*
* Description:
*   There is more going on here than you can understand yet.
*   This is an exercise in typing and error finding.
*   You will make mistakes.  You will find them.  You will correct them.
*   Type in the example carefully.  You will know what it all does soon.
*
****************************************************************************/
codeSegment4:
    /* Student code goes here */
    // Enable clock to the GPIOC peripheral
    ldr		r2, =RCC
    ldr		r3, =AHBENR
    ldr		r1, =GPIOCEN
    ldr		r0, [r2, r3]
    orrs	r0, r1
    str		r0, [r2, r3]

    // set the mode of pin8 as output
    ldr		r2, =GPIOC
    ldr		r3, =MODER
    ldr		r1, =PIN8MOD
    ldr		r0, [r2, r3]
    orrs	r0, r1
    str		r0, [r2, r3]

    // Turn on LED
    ldr		r2, =GPIOC
    ldr		r3, =ODR
    ldr		r1, =PIN8
    ldr		r0, [r2, r3]

    ldr		r4, =PIN8
    orrs	r0, r4
    str		r0, [r2, r3]

loop_inf:
	bl		delay
	eors	r0, r4
	str		r0, [r2, r3]
	b		loop_inf

delay:
	ldr		r1, =6000000
delay_loop:
	subs	r1, r1, #1
	bne		delay_loop
	bx lr

	/* End of student code */
