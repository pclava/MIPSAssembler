.macro blt %r1 %r2 %lbl
    slt $at %r1 %r2
    bne $at $0 %lbl
.end_macro

.macro bge %r1 %r2 %lbl
    slt $at %r1 %r2
    beq $at $0 %lbl
.end_macro

.macro bgt %r1 %r2 %lbl
    slt $at %r2 %r1
    bne $at $0 %lbl
.end_macro

.macro ble %r1 %r2 %lbl
    slt $at %r2 %r1
    beq $at $0 %lbl
.end_macro

.macro move %r1 %r2
    addu %r1 $0 %r2
.end_macro

.macro b %lbl
    beq $0 $0 %lbl
.end_macro