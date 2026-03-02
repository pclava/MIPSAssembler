# Syscall function library
# Generated from C using https://godbolt.org/
.globl __sys_print_int __sys_print_str __sys_read_int
.globl __sys_read_str __sys_print_hex __sys_print_char
.globl __sys_exit __sys_abort __sys_read_mem
.globl __sys_write_mem __sys_open __sys_close
.globl __sys_read __sys_write
__sys_print_int:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li      $v0,1
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_print_str:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li      $v0,4
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_read_int:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        li      $v0,5
        syscall
        sw      $v0,8($fp)
        lw      $2,8($fp)
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        jr      $31
__sys_read_str:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        sw      $5,12($fp)
        li      $v0,8
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_print_hex:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li      $v0,20
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_print_char:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        move    $2,$4
        sb      $2,8($fp)
        li      $v0,11
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_read_mem:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        sw      $4,24($fp)
        lw      $t0,24($fp)
        lbu     $t1,0($t0)
        sb      $t1,8($fp)
        lbu     $2,8($fp)
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        jr      $31
__sys_write_mem:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        move    $2,$5
        sb      $2,12($fp)
        lw      $t0,8($fp)
        lbu     $t1,12($fp)
        sb      $t1,0($t0)
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_exit:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li      $v0,17
        lw      $a0,8($fp)
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_abort:
        addiu   $sp,$sp,-32
        sw      $31,28($sp)
        sw      $fp,24($sp)
        move    $fp,$sp
        li      $4,255                  # 0xff
        jal     __sys_exit
        nop

        nop
        move    $sp,$fp
        lw      $31,28($sp)
        lw      $fp,24($sp)
        addiu   $sp,$sp,32
        jr      $31
        nop

__sys_open:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        sw      $4,24($fp)
        sw      $5,28($fp)
        li      $v0,13
        syscall
        sw      $v0,8($fp)
        lw      $2,8($fp)
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        jr      $31
__sys_close:
        addiu   $sp,$sp,-8
        sw      $fp,4($sp)
        move    $fp,$sp
        sw      $4,8($fp)
        li      $v0,16
        syscall
        nop
        move    $sp,$fp
        lw      $fp,4($sp)
        addiu   $sp,$sp,8
        jr      $31
__sys_read:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        sw      $4,24($fp)
        sw      $5,28($fp)
        sw      $6,32($fp)
        li      $v0,14
        syscall
        sw      $v0,8($fp)
        lw      $2,8($fp)
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        jr      $31
__sys_write:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        sw      $4,24($fp)
        sw      $5,28($fp)
        sw      $6,32($fp)
        li      $v0,15
        syscall
        sw      $v0,8($fp)
        lw      $2,8($fp)
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        jr      $31