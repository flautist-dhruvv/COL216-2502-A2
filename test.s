.data : 10 20 30 40 5 3 2 1
addi x1, x0, 0
addi x2, x0, 4
lw x3, 0(x1)
lw x4, 4(x1)
add x5, x5, x3
mul x6, x6, x4
addi x1, x1, 1
bne x1, x2, 2
div x7, x5, x4
rem x8, x5, x4
sub x9, x5, x6
and x10, x5, x6
or x11, x5, x6
xor x12, x5, x6
andi x13, x5, 15
ori x14, x5, 255
xori x15, x5, 128
slt x16, x5, x6
slti x17, x5, 100
sw x5, 0(x0)
sw x6, 4(x0)
beq x5, x6, 23
addi x18, x0, 1
blt x5, x6, 25
addi x19, x0, 2
ble x5, x6, 27
addi x20, x0, 3
j 29
addi x21, x0, 999
add x0, x0, x0
