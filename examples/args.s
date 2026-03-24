# Demonstration of command line arguments with __start.asm
# Build with:
# MIPSAssembler <path>/args.s
# MIPSLinker <path>/args.s -ls

    .text
    .globl main
main:
    move    $s0, $a0                # Save argc
    move    $s1, $a1                # Save argv
    move    $t0, $zero              # Initialize counter
loop:
    beq     $t0, $s0, endloop       # Branch if equal to argc

    # Print argv[i]
    sll     $t1, $t0, 2             # Multiply counter by 4
    addu    $t2, $s1, $t1           # Add to argv
    lw      $a0, 0($t2)             # Get argv[i] (pointer to string)
    li      $v0, _SYSPRINTSTR       # Print string
    syscall

    # Print space
    li      $v0, _SYSPRINTCHR
    li      $a0, 0x20
    syscall

    addiu   $t0, $t0, 1             # Increment counter
    j       loop                    # Loop
endloop:
    move    $a0, $s0                # Exit with the code equal to argc
    li      $v0, _SYSEXIT2
    syscall