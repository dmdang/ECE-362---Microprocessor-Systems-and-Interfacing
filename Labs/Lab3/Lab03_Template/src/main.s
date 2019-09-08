.syntax unified
        .cpu cortex-m0
        .fpu softvfp
        .thumb

/******************************************************************************
* ECE 362 Lab Experiment 3
* General Purpose I/O
*
******************************************************************************/

.equ  RCC,      0x40021000
.equ  AHBENR,   0x14
.equ  GPIOCEN,  0x00080000
.equ  GPIOBEN,  0x00040000
.equ  GPIOC,    0x48000800
.equ  GPIOB,    0x48000400
.equ  MODER,    0x00
.equ  IDR,      0x10
.equ  ODR,      0x14
.equ  PC0,      0x01
.equ  PC1,      0x04
.equ  PC2,      0x10
.equ  PC3,      0x40
.equ  PIN8,     0x00000100


//===========================================================
// Enable Ports B and C in the RCC AHBENR
// No parameters.
// No expected return value.
.global enable_ports
enable_ports:
    push    {lr}
    /* Student code goes here */

    ldr r1, =RCC
	ldr r3, [r1, #AHBENR]
	ldr r4, =0xc0000
	orrs r3, r3, r4
	str r3, [r1, #AHBENR]

    /* End of student code*/
    pop     {pc}

//===========================================================
// Set pins 0-3 of Port C to be outputs.
// No parameters.
// No expected return value.
.global port_c_output
port_c_output:
    push    {lr}
    /* Student code goes here */

	ldr r1, =GPIOC
	ldr r3, [r1, #MODER]
	ldr r4, =0x55
	orrs r3, r3, r4
	str r3, [r1, #MODER]

    /* End of student code*/
    pop     {pc}

//===========================================================
// Set the state of a single output pin to be high.
// Do not affect the other bits of the port.
// Parameter 1 is the GPIOx base address.
// Parameter 2 is the bit number of the pin.
// No expected return value.
.global setpin
setpin:
    push    {lr}
    /* Student code goes here */

	ldr r3, [r0, #ODR]
	movs r2, 0x1
	lsls r2, r1
	orrs r3, r3, r2
	str r3, [r0, #ODR]

    /* End of student code*/
    pop     {pc}

//===========================================================
// Set the state of a single output pin to be low.
// Do not affect the other bits of the port.
// Parameter 1 is the GPIOx base address.
// Parameter 2 is the bit number of the pin.
// No expected return value.
.global clrpin
clrpin:
    push    {lr}
    /* Student code goes here */

	ldr r3, [r0, #ODR]
    movs r2, 0x1
    lsls r2, r1
    mvns r2, r2
    ands r3, r3, r2		// use ands, but make it so it doesn't mess with other bits
	str r3, [r0, #ODR]

    /* End of student code*/
    pop     {pc}

//===========================================================
// Get the state of the input data register of
// the specified GPIO.
// Parameter 1 is GPIOx base address.
// Parameter 2 is the bit number of the pin.
// The subroutine should return 0x1 if the pin is high
// or 0x0 if the pin is low.
.global getpin
getpin:
    push    {lr}
    /* Student code goes here */

    ldr r3, [r0, #IDR]
    movs r2, #0x1
    lsls r2, r1
    ands r3, r3, r2
    lsrs r3, r1
    movs r0, r3

    /* End of student code*/
    pop     {pc}

//===========================================================
// Get the state of the input data register of
// the specified GPIO.
// Parameter 1 is GPIOx base address.
// Parameter 2 is the direction of the shift
//
// Perfroms the following logic
// 1) Read the current content of GPIOx-ODR
// 2) If R1 = 1
//      (a) Left shift the content by 1
//      (b) Check if value exceeds 8
//      (c) If so set it to 0x1
// 3) If R1 != 0
//      (a) Right shift the content by 1
//      (b) Check if value is 0
//      (c) If so set it to 0x8
// 4) Store the new value in ODR
// No return value
.global seq_led
seq_led:
    push    {lr}
    /* Student code goes here */

    ldr r3, [r0, #ODR]
    movs r2, #0x1
    cmp r1, r2
    beq equalOne
    bne nequalOne

equalOne:

	lsls r3, r3, #1
	cmp r3, #0x8
	bgt gtEight
	str r3, [r0, #ODR]
	b end

gtEight:

	movs r3, #0x1
	str r3, [r0, #ODR]
	b end

nequalOne:

	lsrs r3, r3, #1
	cmp r3, #0
	beq eqZero
	str r3, [r0, #ODR]
	b end

eqZero:

	movs r3, #0x8
	str r3, [r0, #ODR]
	b end

    /* End of student code*/
end:
    pop     {pc}

.global main
main:
    // Uncomment the line below to test "enable_ports"
    //bl  test_enable_ports

    // Uncomment the line below to test "port_c_output"
    //bl  test_port_c_output

    // Uncomment the line below to test "setpin and clrpin"
    //bl  test_set_clrpin

    // Uncomment the line below to test "getpin"
    //bl  test_getpin

    // Uncomment the line below to test "getpin"
  //bl  test_wiring

    // Uncomment to run the LED sequencing program
    //bl run_seq

inf_loop:
    b inf_loop
