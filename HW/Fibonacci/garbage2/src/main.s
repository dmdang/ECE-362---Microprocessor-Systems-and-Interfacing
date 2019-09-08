.cpu cortex-m0
.thumb
.syntax unified
.fpu softvfp

.text
.global main
.global fibonacci

main:

    movs r0, #15
    bl fibonacci
    wfi

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
