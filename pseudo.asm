# Pseudoinstructions inserted at the start of every file by the preprocessor

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