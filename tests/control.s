.section .text, "ax"
.global main

main:
cbnz w0, tg1
mov x1, x0
tg1:
tbnz x0, #16, tg2
mov x1, x2
tg2:
b tg3
mov x1, x2
tg3:
bl tg4
ldr x0, =tg5
blr x0
b exit

tg4:
mov x3, x4
ret

tg5:
mov x5, x6
ret

exit:
mov x0, #12
mov x8, #93
svc 0

