# Dynamic memory allocator
# Compiled from C with Compiler Explorer

# Depends on system.s

# Implements malloc, free, calloc, and realloc with the same signatures
# as in the C standard library

.globl malloc free calloc
.globl realloc

set_size:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        sw      $4,24($fp)
        sw      $5,28($fp)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        sw      $2,8($fp)
        lw      $2,24($fp)
        lw      $3,0($2)
        lw      $2,8($fp)
        nor     $2,$0,$2
        and     $3,$3,$2
        lw      $2,24($fp)
        sw      $3,0($2)
        lw      $2,24($fp)
        lw      $3,0($2)
        lw      $4,8($fp)
        lw      $2,28($fp)
        and     $2,$4,$2
        or      $3,$3,$2
        lw      $2,24($fp)
        sw      $3,0($2)
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        jr      $31
set_free:
        addiu   $sp,$sp,-24
        sw      $fp,20($sp)
        move    $fp,$sp
        sw      $4,24($fp)
        move    $2,$5
        sb      $2,28($fp)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        sw      $2,8($fp)
        lw      $2,24($fp)
        lw      $3,0($2)
        lw      $2,8($fp)
        and     $3,$3,$2
        lw      $2,24($fp)
        sw      $3,0($2)
        lw      $2,24($fp)
        lw      $3,0($2)
        lbu     $2,28($fp)
        sll     $2,$2,31
        or      $3,$3,$2
        lw      $2,24($fp)
        sw      $3,0($2)
        move    $sp,$fp
        lw      $fp,20($sp)
        addiu   $sp,$sp,24
        jr      $31
.data
free_chunks:
        .space  4
heap_size:
        .word   131072
.text
initialize_heap:
        addiu   $sp,$sp,-32
        sw      $31,28($sp)
        sw      $fp,24($sp)
        move    $fp,$sp
        li      $4,131072             # 0x20000
        jal     __sys_sbrk
        move    $3,$2
        lui     $2,%hi(free_chunks)
        sw      $3,%lo(free_chunks)($2)
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        li      $3,-2147352576                  # 0xffffffff80020000
        sw      $3,0($2)
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        sw      $0,8($2)
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        sw      $0,12($2)
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        lui     $3,%hi(free_chunks)
        lw      $3,%lo(free_chunks)($3)
        sw      $3,4($2)
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        move    $sp,$fp
        lw      $31,28($sp)
        lw      $fp,24($sp)
        addiu   $sp,$sp,32
        jr      $31
find_free:
        addiu   $sp,$sp,-40
        sw      $31,36($sp)
        sw      $fp,32($sp)
        move    $fp,$sp
        sw      $4,40($fp)
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        bne     $2,$0,$L6
        jal     initialize_heap
        j       $L7
$L6:
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        sw      $2,24($fp)
        j       $L8
$L10:
        lw      $2,24($fp)
        lw      $2,8($2)
        sw      $2,24($fp)
$L8:
        lw      $2,24($fp)
        lw      $2,8($2)
        beq     $2,$0,$L9
        lw      $2,24($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$3,$2
        lw      $3,40($fp)
        sltu    $2,$2,$3
        bne     $2,$0,$L10
$L9:
        lw      $2,24($fp)
$L7:
        move    $sp,$fp
        lw      $31,36($sp)
        lw      $fp,32($sp)
        addiu   $sp,$sp,40
        jr      $31
malloc:
        addiu   $sp,$sp,-64
        sw      $31,60($sp)
        sw      $fp,56($sp)
        move    $fp,$sp
        sw      $4,64($fp)
        lw      $2,64($fp)
        bne     $2,$0,$L12
        move    $2,$0
        j       $L13
$L12:
        lw      $2,64($fp)
        addiu   $2,$2,16
        sw      $2,24($fp)
        j       $L14
$L15:
        lw      $2,24($fp)
        addiu   $2,$2,1
        sw      $2,24($fp)
$L14:
        lw      $2,24($fp)
        andi    $2,$2,0x3
        bne     $2,$0,$L15
        lw      $4,24($fp)
        jal     find_free
        sw      $2,32($fp)
        lw      $2,32($fp)
        bne     $2,$0,$L16
        move    $2,$0
        j       $L13
$L16:
        lw      $2,32($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $3,$3,$2
        lw      $2,24($fp)
        subu    $2,$3,$2
        sw      $2,36($fp)
        lw      $2,36($fp)
        bgez    $2,$L17
        move    $2,$0
        j       $L13
$L17:
        lw      $2,32($fp)
        lw      $2,8($2)
        sw      $2,40($fp)
        lw      $2,32($fp)
        lw      $2,12($2)
        sw      $2,44($fp)
        lw      $2,40($fp)
        beq     $2,$0,$L18
        lw      $2,32($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$3,$2
        lw      $3,32($fp)
        addu    $2,$3,$2
        sw      $2,28($fp)
        j       $L19
$L18:
        sw      $0,28($fp)
$L19:
        lw      $2,40($fp)
        bne     $2,$0,$L20
        lw      $2,36($fp)
        sltiu   $2,$2,17
        beq     $2,$0,$L20
        move    $2,$0
        j       $L13
$L20:
        lw      $2,36($fp)
        bne     $2,$0,$L21
        lw      $2,40($fp)
        beq     $2,$0,$L22
        lw      $2,28($fp)
        bne     $2,$0,$L23
$L22:
        move    $2,$0
        j       $L13
$L23:
        lw      $2,44($fp)
        beq     $2,$0,$L24
        lw      $2,40($fp)
        lw      $3,8($2)
        lw      $2,44($fp)
        sw      $3,8($2)
        j       $L25
$L24:
        lui     $2,%hi(free_chunks)
        lw      $3,40($fp)
        sw      $3,%lo(free_chunks)($2)
$L25:
        lw      $2,32($fp)
        lw      $3,28($fp)
        sw      $3,8($2)
        j       $L26
$L27:
        lw      $2,28($fp)
        lw      $3,44($fp)
        sw      $3,12($2)
        lw      $2,28($fp)
        lw      $2,8($2)
        sw      $2,28($fp)
$L26:
        lw      $2,28($fp)
        lw      $2,0($2)
        bgez    $2,$L27
        lw      $2,40($fp)
        lw      $3,44($fp)
        sw      $3,12($2)
        j       $L28
$L21:
        lw      $2,32($fp)
        lw      $3,4($2)
        lw      $2,24($fp)
        addu    $2,$3,$2
        sw      $2,48($fp)
        lw      $2,32($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $3,$3,$2
        lw      $2,24($fp)
        subu    $2,$3,$2
        sw      $2,52($fp)
        lw      $3,52($fp)
        li      $2,-2147483648                  # 0xffffffff80000000
        or      $3,$3,$2
        lw      $2,48($fp)
        sw      $3,0($2)
        lw      $2,48($fp)
        lw      $3,48($fp)
        sw      $3,4($2)
        lw      $2,44($fp)
        beq     $2,$0,$L29
        lw      $2,44($fp)
        lw      $3,48($fp)
        sw      $3,8($2)
        j       $L30
$L29:
        lui     $2,%hi(free_chunks)
        lw      $3,48($fp)
        sw      $3,%lo(free_chunks)($2)
$L30:
        lw      $2,32($fp)
        lw      $3,48($fp)
        sw      $3,8($2)
        lw      $2,48($fp)
        lw      $3,40($fp)
        sw      $3,8($2)
        lw      $2,48($fp)
        lw      $3,44($fp)
        sw      $3,12($2)
        j       $L31
$L33:
        lw      $2,28($fp)
        lw      $3,48($fp)
        sw      $3,12($2)
        lw      $2,28($fp)
        lw      $2,8($2)
        sw      $2,28($fp)
$L31:
        lw      $2,28($fp)
        beq     $2,$0,$L32
        lw      $2,28($fp)
        lw      $2,0($2)
        bgez    $2,$L33
$L32:
        lw      $2,40($fp)
        beq     $2,$0,$L28
        lw      $2,40($fp)
        lw      $3,48($fp)
        sw      $3,12($2)
$L28:
        lw      $3,24($fp)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $3,$3,$2
        lw      $2,32($fp)
        sw      $3,0($2)
        lw      $2,32($fp)
        addiu   $2,$2,16
$L13:
        move    $sp,$fp
        lw      $31,60($sp)
        lw      $fp,56($sp)
        addiu   $sp,$sp,64
        jr      $31
free:
        addiu   $sp,$sp,-64
        sw      $31,60($sp)
        sw      $fp,56($sp)
        move    $fp,$sp
        sw      $4,64($fp)
        lw      $2,64($fp)
        beq     $2,$0,$L49
        lw      $2,64($fp)
        addiu   $2,$2,-16
        sw      $2,32($fp)
        lw      $2,32($fp)
        lw      $2,0($2)
        bltz    $2,$L34
        lw      $2,32($fp)
        lw      $2,12($2)
        sw      $2,36($fp)
        lw      $2,36($fp)
        beq     $2,$0,$L38
        lw      $2,36($fp)
        lw      $2,8($2)
        sw      $2,24($fp)
        j       $L39
$L38:
        lui     $2,%hi(free_chunks)
        lw      $2,%lo(free_chunks)($2)
        sw      $2,24($fp)
$L39:
        lw      $2,32($fp)
        lw      $2,8($2)
        sw      $2,28($fp)
        li      $5,1                        # 0x1
        lw      $4,32($fp)
        jal     set_free
        lw      $2,36($fp)
        beq     $2,$0,$L40
        lw      $2,36($fp)
        lw      $3,32($fp)
        sw      $3,8($2)
$L40:
        lw      $2,32($fp)
        lw      $3,24($fp)
        sw      $3,8($2)
        j       $L41
$L42:
        lw      $2,28($fp)
        lw      $3,32($fp)
        sw      $3,12($2)
        lw      $2,28($fp)
        lw      $2,8($2)
        sw      $2,28($fp)
$L41:
        lw      $3,28($fp)
        lw      $2,24($fp)
        bne     $3,$2,$L42
        lw      $2,24($fp)
        lw      $3,32($fp)
        sw      $3,12($2)
        lw      $2,32($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$3,$2
        lw      $3,32($fp)
        addu    $2,$3,$2
        lw      $3,24($fp)
        bne     $3,$2,$L43
        lw      $2,32($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $3,$3,$2
        lw      $2,24($fp)
        lw      $4,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$4,$2
        addu    $2,$3,$2
        sw      $2,40($fp)
        lw      $5,40($fp)
        lw      $4,32($fp)
        jal     set_size
        lw      $2,24($fp)
        lw      $3,8($2)
        lw      $2,32($fp)
        sw      $3,8($2)
        lw      $2,24($fp)
        lw      $2,8($2)
        beq     $2,$0,$L43
        lw      $2,24($fp)
        lw      $2,8($2)
        sw      $2,44($fp)
        lw      $2,24($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$3,$2
        lw      $3,24($fp)
        addu    $2,$3,$2
        sw      $2,28($fp)
        j       $L44
$L45:
        lw      $2,28($fp)
        lw      $3,32($fp)
        sw      $3,12($2)
        lw      $2,28($fp)
        lw      $2,8($2)
        sw      $2,28($fp)
$L44:
        lw      $3,28($fp)
        lw      $2,44($fp)
        bne     $3,$2,$L45
        lw      $2,44($fp)
        lw      $3,32($fp)
        sw      $3,12($2)
$L43:
        lw      $2,36($fp)
        beq     $2,$0,$L46
        lw      $2,36($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$3,$2
        lw      $3,36($fp)
        addu    $2,$3,$2
        lw      $3,32($fp)
        bne     $3,$2,$L46
        lw      $2,36($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $3,$3,$2
        lw      $2,32($fp)
        lw      $4,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$4,$2
        addu    $2,$3,$2
        sw      $2,48($fp)
        lw      $5,48($fp)
        lw      $4,36($fp)
        jal     set_size
        lw      $2,32($fp)
        lw      $3,8($2)
        lw      $2,36($fp)
        sw      $3,8($2)
        lw      $2,32($fp)
        lw      $2,8($2)
        beq     $2,$0,$L46
        lw      $2,32($fp)
        lw      $2,8($2)
        sw      $2,52($fp)
        lw      $2,32($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$3,$2
        lw      $3,32($fp)
        addu    $2,$3,$2
        sw      $2,28($fp)
        j       $L47
$L48:
        lw      $2,28($fp)
        lw      $3,36($fp)
        sw      $3,12($2)
        lw      $2,28($fp)
        lw      $2,8($2)
        sw      $2,28($fp)
$L47:
        lw      $3,28($fp)
        lw      $2,52($fp)
        bne     $3,$2,$L48
        lw      $2,52($fp)
        lw      $3,36($fp)
        sw      $3,12($2)
$L46:
        lw      $2,36($fp)
        bne     $2,$0,$L34
        lui     $2,%hi(free_chunks)
        lw      $3,32($fp)
        sw      $3,%lo(free_chunks)($2)
        j       $L34
$L49:
        j       $L34
$L34:
        move    $sp,$fp
        lw      $31,60($sp)
        lw      $fp,56($sp)
        addiu   $sp,$sp,64
        jr      $31
calloc:
        addiu   $sp,$sp,-32
        sw      $31,28($sp)
        sw      $fp,24($sp)
        move    $fp,$sp
        sw      $4,32($fp)
        sw      $5,36($fp)
        lw      $3,32($fp)
        lw      $2,36($fp)
        mult    $3,$2
        mflo    $4
        jal     malloc
        move    $sp,$fp
        lw      $31,28($sp)
        lw      $fp,24($sp)
        addiu   $sp,$sp,32
        jr      $31
realloc:
        addiu   $sp,$sp,-56
        sw      $31,52($sp)
        sw      $fp,48($sp)
        move    $fp,$sp
        sw      $4,56($fp)
        sw      $5,60($fp)
        lw      $2,56($fp)
        bne     $2,$0,$L54
        lw      $4,60($fp)
        jal     malloc
        j       $L55
$L54:
        lw      $2,56($fp)
        addiu   $2,$2,-16
        sw      $2,24($fp)
        lw      $2,24($fp)
        lw      $2,0($2)
        bgez    $2,$L56
        move    $2,$0
        j       $L55
$L56:
        lw      $2,60($fp)
        bne     $2,$0,$L57
        lw      $4,56($fp)
        jal     free
        move    $2,$0
        j       $L55
$L57:
        lw      $2,60($fp)
        addiu   $2,$2,16
        sw      $2,28($fp)
        lw      $2,24($fp)
        lw      $3,0($2)
        li      $2,2147418112                 # 0x7fff0000
        ori     $2,$2,0xffff
        and     $2,$3,$2
        sw      $2,32($fp)
        lw      $3,28($fp)
        lw      $2,32($fp)
        sltu    $2,$2,$3
        bne     $2,$0,$L58
        lw      $2,56($fp)
        j       $L55
$L58:
        lw      $4,56($fp)
        jal     free
        lw      $4,60($fp)
        jal     malloc
        sw      $2,36($fp)
        lw      $2,56($fp)
        sw      $2,40($fp)
        lw      $2,32($fp)
        addiu   $2,$2,-16
        move    $6,$2
        lw      $5,40($fp)
        lw      $4,36($fp)
        jal     memcpy
        lw      $2,36($fp)
$L55:
        move    $sp,$fp
        lw      $31,52($sp)
        lw      $fp,48($sp)
        addiu   $sp,$sp,56
        jr      $31