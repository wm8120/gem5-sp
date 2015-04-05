//test gem5 trace on memory acess, especially for the stride and
// ldr/str pair registers
//
// Meng Wang
// Feb. 16th, 2015

.section .text, "ax"
.global main

main:
//mov w1, #0
//strb w1, [sp], -1 // stride = 1, data = 0
//mov w1, #1
//strb w1, [sp], -1 // stride = 1, data = 1
ldr x0, =test_data  
ldr x1, [x0], #8
ldr x1, [x0]
stp x1, x2, [sp, #-16] // stride = 8
ldp x3, x4, [sp, #-16] // stride = 8
stp x3, x4, [sp, #-16] // stride = 8
ldp w1, w0, [sp, #-16] // stride = 8
stp w1, w0, [sp, #-16] // stride = 8
sub sp, sp, #16
// byte test
ldr x1, [sp, #-16]
ldrb w1, [sp]       // stride = 1, data = 0
strb w1, [sp, #-16] // stride = 1, data = 0
ldr x1, [sp, #-16] 
ldrb w2, [sp, #1]   // stride = 1, data = 1
strb w2, [sp, #-15] // stride = 1, data = 1
ldr x1, [sp, #-16]
//half word test
ldr x1, [sp, #-16] 
ldrh w2, [sp, #2] // stride = 2, data = 0x0403
strh w2, [sp, #-14] // stride = 2, data = 0x0403
ldr x1, [sp, #-16]
//word test
ldr x1, [sp, #-16]
ldr w1, [sp, #3]  // stride = 4, data = 0x06050403
ldr w2, [sp, #4]  // stride = 4, data = 0x07060504
str w1, [sp, #-13] // stride = 4, data = 0x05040302
ldr x1, [sp, #-16]
//ldrsb wt, addr; ldrsh xt, addr; ldrsw xt, addr
ldrsb w1, [sp, #3] // x1 = 0x0000000000000002
ldrsh x1, [sp, #4] // x1 = 0x0000000000000403
ldrsw x1, [sp, #6] // x1 = 0x0000000008070605
//stxp ws, wt1, wt2, [base,{#0}]
ldr x0, [sp, #-8]
ldp w1, w2, [sp, #8]
sub x4, sp, #8
stxp w3, w1, w2, [x4]
ldr x5, [sp, #-8]
ldp x1, x2, [sp, #-16]

// exit
mov x0, #0 
mov x8, #93 // exit
svc 0

test_data:
.quad 0x0706050403020100
.quad 0x0f0e0d0c0b0a0908

