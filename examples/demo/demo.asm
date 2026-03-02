# Demo using multiple files
# Assembled with: ./MIPSAssembler system.asm demo.asm -o demo.out
# Compiled from C using https://godbolt.org/
.data
$LC0:
        .ascii  "Enter a string: \000"
$LC1:
        .ascii  "Enter a number: \000"
.text
.globl main
main:
        addiu   $sp,$sp,-296
        sw      $31,292($sp)
        sw      $fp,288($sp)
        move    $fp,$sp
        sw      $4,296($fp)
        sw      $5,300($fp)
        lui     $2,%hi($LC0)
        addiu   $4,$2,%lo($LC0)
        jal     __sys_print_str
        nop

        addiu   $2,$fp,32
        li      $5,256                  # 0x100
        move    $4,$2
        jal     __sys_read_str
        nop

        lui     $2,%hi($LC1)
        addiu   $4,$2,%lo($LC1)
        jal     __sys_print_str
        nop

        jal     __sys_read_int
        nop

        sw      $2,28($fp)
        sw      $0,24($fp)
        b       $L2
        nop

$L3:
        addiu   $2,$fp,32
        move    $4,$2
        jal     __sys_print_str
        nop

        li      $4,10                 # 0xa
        jal     __sys_print_char
        nop

        lw      $2,24($fp)
        nop
        addiu   $2,$2,1
        sw      $2,24($fp)
$L2:
        lw      $3,24($fp)
        lw      $2,28($fp)
        nop
        slt     $2,$3,$2
        bne     $2,$0,$L3
        nop

        li      $4,78                 # 0x4e
        jal     __sys_exit
        nop

        move    $2,$0
        move    $sp,$fp
        lw      $31,292($sp)
        lw      $fp,288($sp)
        addiu   $sp,$sp,296
        jr      $31
        nop
