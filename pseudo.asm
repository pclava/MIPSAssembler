# Macros inserted at the start of every file by the preprocessor

.define _TEXT 0x00400000
.define _DATA 0x10010000
.define _STACK 0x7fffffff
.define _HEAP 0x10080000
.define _KTEXT 0x80000000
.define _KDATA 0x90000000

# Push to stack
.macro push %r
    addiu $sp, $sp, -4
    sw %r, 0($sp)
.end_macro

# Pop from stack
.macro pop %r
    lw %r, 0($sp)
    addiu $sp, $sp, 4
.end_macro

# Branch less than
.macro blt %r1 %r2 %lbl
    slt $at, %r1, %r2
    bne $at, $0, %lbl
.end_macro

# Branch greater or equal
.macro bge %r1 %r2 %lbl
    slt $at, %r1, %r2
    beq $at, $0, %lbl
.end_macro

# Branch greater
.macro bgt %r1 %r2 %lbl
    slt $at, %r2, %r1
    bne $at, $0, %lbl
.end_macro

# Branch less or equal
.macro ble %r1 %r2 %lbl
    slt $at, %r2, %r1
    beq $at, $0, %lbl
.end_macro

# Move
.macro move %r1 %r2
    addu %r1, $0, %r2
.end_macro

# Short-distance unconditional branch
.macro b %lbl
    beq $0, $0, %lbl
.end_macro

# Load address
.macro la %r %lbl
    lui $at, %hi(%lbl)
    ori %r, $at, %lo(%lbl)
.end_macro

# Terminate program with exit code 0
.macro done
    li $v0, 10
    syscall
.end_macro

# Terminate program with an exit code
.macro exit %imm
    li $v0, 17
    li $a0, %imm
    syscall
.end_macro