# Demonstration of some directives

.eqv newline 0x0a

.macro printchr %imm
    li  $v0, 4
    li  $a0, %imm
    syscall
.end_macro

.data
string: .asciiz "Hello, World!"

.text
# Print the string
la  $a0, string
li  $v0, 4
syscall

# Print a new line, using the symbolic constant and macro
printchr newline

# Exit
li  $v0, 10
syscall