# Optionally inserted by the linker at the end of the program - calls the 'main' function

.text
.globl __start
__start:
    move    $fp, $sp        # Set frame pointer

    # main(argc, argv)
    lw      $a0, 0($sp)     # argc = sp[0]
    addiu   $a1, $sp, 4     # argv starts at sp[4]
    addiu   $sp, $sp, -8    # Move stack to fit arguments
    jal     main

    move    $a0 $v0         # Put return value in a0 (argument to syscall 17)

__exit:
    li $v0 17           # Exit with the value in a0
    syscall