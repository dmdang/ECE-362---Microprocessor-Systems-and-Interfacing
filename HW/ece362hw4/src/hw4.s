.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

//============================================================================
// Q1: hello
//============================================================================
.global hello
hello:
	push {lr}
	ldr r0, =greeting
	bl printf
	pop {pc}

greeting:
	.string "Hello, World!\n"
	.align 2
//============================================================================
// Q2: add2
//============================================================================
.global add2
add2:

	push {r4, r5, lr}

	adds r2, r0, r1 // sum in r2, r0 = a, r1 = b
	movs r3, r0 // r3 is a
	movs r4, r1 // r4 is b
	movs r5, r2 // r5 is sum
	ldr r0, =format // r0 is format
	movs r1, r3 // r1 is a
	movs r2, r4
	movs r3, r5 // r3 is
	bl printf

	pop {r4, r5, pc}


format:
	.string "%d + %d = %d\n"
	.align 2

//============================================================================
// Q3: add3
//============================================================================
.global add3
add3:
	push {r4-r7, lr}

	sub sp, #4
	adds r7, r0, r1 // r7 is sum
	adds r7, r2
	movs r4, r0 // r4 is a
	movs r5, r1 // r5 is b
	movs r6, r2 // r6 is c
	ldr r0, =format2 // r0 is string
	movs r1, r4
	movs r2, r5
	movs r3, r6
	movs r4, r7
	str r4, [sp, #0]
	bl printf
	add sp, #4

	pop {r4-r7, pc}

format2:
	.string "%d + %d + %d = %d\n"
	.align 2
//============================================================================
// Q4: rotate6
//============================================================================
.global rotate6
rotate6:
	push {r4-r7, lr}

rotate6n:
	cmp r0, #0
	bne notequal
	beq equal

notequal:
	movs r7, r0 // r7 is a
	movs r6, r1 // r6 is b
	ldr r0, [sp, #24] // r0 is f ^
	movs r1, r7 // r1 is a ^
	movs r7, r2 // r7 is c
	movs r2, r6 // r2 is b ^
	movs r6, r3 // r6 is d
	movs r3, r7 // r3 is c ^
	ldr r7, [sp, #20] // r7 is e
	str r6, [sp, #20] // r4 is d ^
	ldr r6, [sp, #24] // r6 is f
	str r7, [sp, #24] // r5 is e ^
	b rotate6n
	b end

equal:
	ldr r7, [sp, #24]
	ldr r6, [sp, #20]
	subs r7, r6
	subs r7, r3
	subs r7, r2
	subs r7, r1
	subs r7, r0
	movs r0, r7
	b end

end:
	pop {r4-r7, pc}

//============================================================================
// Q5: low_pattern
//============================================================================
.type compare, %function  // You knew you needed this line.  Of course you did!
compare:
        ldr  r0,[r0]
        ldr  r1,[r1]
        subs r0,r1
        bx lr

.global low_pattern
low_pattern:

	push {r4-r7, lr}
	// r0 is nth
	movs r5, r0 // r5 is now nth
	sub sp, #400
	mov r7, sp // r7 is beginning of array
	movs r4, #0 // r4 is x

	movs r0, #127
	movs r1, #0xff


for1:
	cmp r4, #100
	bge endloop

	adds r6, r4, #1
	muls r6, r6, r0
	ands r6, r6, r1

	lsls r2, r4, #2

	str r6, [r7, r2]

	adds r4, r4, #1
	b for1

endloop:
	movs r0, r7 // array
	movs r1, #100 // 100
	movs r2, #4 // 4
	ldr r3, =compare // compare
	bl qsort

	lsls r4, r5, #2

	ldr r0, [r7, r4]
	add sp, #400
	pop {r4-r7, pc}

//============================================================================
// Q6: get_name
//============================================================================
.global get_name
get_name:

	push {r4-r7, lr}
	sub sp, #100 // allocate space for char buffer
	mov r7, sp // r7 points to array
	//ldr r2, =format3
	ldr r0, =format3 // load first string into r0
	bl printf // "Enter your name: "

	mov r0, r7 // make r0 point to char buffer
	bl gets

	ldr r0, =format4
	mov r1, sp
	bl printf

	add sp, #100 // deallocate space
	pop {r4-r7, pc}

format3:
	.string "Enter your name: "
	.align 2
format4:
	.string "Hello, %s\n"
	.align 2

//============================================================================
// Q7: random_sum
//============================================================================
.global random_sum
random_sum:
	push {r4-r7, lr}

	sub sp, #80 // allocate array for 20 ints
	movs r4, #1 // x
	movs r5, #0 // sum
	mov r7, sp // r7 points to array

	bl random
	str r0, [r7, #0] // arr[0] = random();

for2:
	cmp r4, #20
	bge moveZero
	movs r6, r4 // use r6 as r4 - 1
	subs r6, r6, #1

	bl random // result is in r0
	lsls r6, r6, #2
	ldr r1, [r7, r6]
	adds r0, r0, r1 // arr[x - 1] + random();
	lsls r2, r4, #2
	str r0, [r7, r2] // arr[x] = "..."

	adds r4, r4, #1
	b for2

moveZero:
	movs r4, #0
	b for3

for3:
	cmp r4, #20
	bge endloop2

	lsls r6, r4, #2
	ldr r2, [r7, r6]
	adds r5, r5, r2

	adds r4, r4, #1
	b for3

endloop2:
	movs r0, r5
	add sp, #80
	pop {r4-r7, pc}

//============================================================================
// Q8: fibn
//============================================================================
.global fibn
fibn:
	push {r4-r7, lr}
	sub sp, #496 // allocate space for arr[124]
	mov r7, sp // sp points to arr
	movs r4, #2 // x
	movs r6, #0
	movs r5, #1
	str r6, [r7, #0]
	str r5, [r7, #4]

for6:
	cmp r4, #124
	bge endloop4
	movs r3, r4 // use r3 as x - 1
	subs r3, r3, #1
	lsls r3, r3, #2
	ldr r2, [r7, r3] // arr[x-1]
	subs r3, r4, #2 // use r3 as x - 2
	lsls r3, r3, #2
	ldr r1, [r7, r3] // arr[x-2]
	adds r1, r1, r2

	movs r6, r4 // use r6 as x
	lsls r6, r6, #2
	str r1, [r7, r6]

	adds r4, r4, #1
	b for6

endloop4:
	movs r5, r0 // r5 is n
	lsls r5, r5, #2
	ldr r0, [r7, r5]
	add sp, #496
	pop {r4-r7, pc}

//============================================================================
// Q9: fun
//============================================================================
.global fun
fun:
	push {r4-r7, lr}

	sub sp, #400 // allocate space for int arr[100]
	mov r7, sp // r7 points to arr
	movs r4, #1 // x
	movs r5, #0 // sum

	movs r6, #0

	str r6, [r7, #0] // arr[0] = 0;

for4:
	cmp r4, #100
	bge moveA
	movs r3, r4 // use r3 as x - 1
	subs r3, r3, #1
	lsls r3, r3, #2
	ldr r2, [r7, r3] // arr[x - 1]
	lsrs r3, r3, #2
	adds r3, r3, #1 // restore r3 as x
	adds r6, r4, #5 // (x + 5)

	push {r4}
	movs r4, #41
	muls r6, r6, r4 // (x + 5) * 41
	pop {r4}

	adds r6, r6, r2 // add both terms
	lsls r3, r3, #2
	str r6, [r7, r3] // arr[x] = "..."
	adds r4, r4, #1
	b for4


moveA:
	movs r4, r0
	b for5

for5:
	cmp r4, r1
	bgt endloop3

	lsls r6, r4, #2
	ldr r2, [r7, r6]

	adds r5, r5, r2 // sum += arr[x]

	adds r4, r4, #1
	b for5

endloop3:
	movs r0, r5
	add sp, #400
	pop {r4-r7, pc}
//============================================================================
// Q10: sick
//============================================================================
.global sick
sick:

	push {r4-r7, lr}
	// argument r4 is on the stack, position 420

	sub sp, #400 // allocate space for arr[100]
	mov r7, sp // r7 points arr
	movs r4, #1 // x
	movs r5, #0 // sum

	movs r6, #0

	str r6, [r7, #0]

for7:
	cmp r4, #100
	bge moveStart

	push {r0, r5}

	movs r6, r4 // use r6 as x - 1
	subs r6, r6, #1
	lsls r6, r6, #2
	ldr r5, [r7, r6] // arr[x - 1]
	lsrs r6, r6, #2
	adds r6, r6, #1 // restore r6 as x
	adds r0, r4, r2 // (x + add)
	muls r0, r0, r3 // (x + add) * mul
	adds r0, r0, r5 // add both terms

	lsls r6, r6, #2
	str r0, [r7, r6] // arr[x] = "..."
	lsrs r6, r6, #2 // restore x

	pop {r0, r5}

	adds r4, r4, #1
	b for7

moveStart:
	movs r4, r0
	b for8

for8:
	cmp r4, r1
	bgt endloop5

	push {r2}

	lsls r6, r4, #2
	ldr r2, [r7, r6] // arr[x]
	adds r5, r5, r2

	pop {r2}

	ldr r6, [sp, #420]
	adds r4, r4, r6
	b for8

endloop5:
	movs r0, r5
	add sp, #400
	pop {r4-r7, pc}
