# Assembled with: ./MIPSAssembler -e. helloworld.asm -o helloworld.out
.data
str: .asciiz "Hello, World!\n"
.text
li $v0, 4   # syscall print string
syscall
li $v0, 10  # syscall exit
syscall