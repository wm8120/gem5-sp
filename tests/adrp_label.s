.section .text, "ax"
.global main

main:
adrp x0, lb1 //comment
adr x1, lb1
mov x8, 93
mov x2, #10
svc 0


lb1:
.word 0x00000000
