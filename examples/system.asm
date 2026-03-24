# Library of syscalls formatted as functions, to be linked to another file

.globl __sys_print_int __sys_print_str __sys_read_int
.globl __sys_read_str __sys_print_hex __sys_print_char
.globl __sys_exit __sys_read_mem __sys_brk
.globl __sys_write_mem __sys_open __sys_close
.globl __sys_read __sys_write __sys_sbrk
__sys_print_int:	# void __sys_print_int(int x)
        li      $v0, _SYSPRINTINT
        syscall
        jr      $ra
__sys_print_str:	# void __sys_print_str(char *str)
        li      $v0, _SYSPRINTSTR
        syscall
        jr      $ra
__sys_read_int:		# int __sys_read_int(void);
        li      $v0, _SYSREADINT
        syscall
        jr      $ra
__sys_read_str:		# void __sys_read_str(char *, int bytes)
        li      $v0, _SYSREADSTR
        syscall
        jr      $ra
__sys_print_hex:	# void __sys_print_hex(int hex)
        li      $v0, _SYSPRINTHEX
        syscall
        jr      $ra
__sys_print_char:	# void __sys_print_char(char c)
        li      $v0, _SYSPRINTCHR
        syscall
        jr      $ra
__sys_read_mem:		# unsigned char __sys_read_mem(unsigned int addr)
        lbu     $v0,0($a0)
        jr      $ra
__sys_write_mem:	# void __sys_write_mem(unsigned int addr, unsigned char value)
        sb      $a1,0($a0)
        jr      $ra
__sys_exit:		# void __sys_exit(int status)
        li      $v0, _SYSEXIT2
        syscall
        jr      $ra
__sys_open:		# int __sys_open(char *path, unsigned int flags)
        li      $v0, _SYSOPENFILE
        syscall
        jr      $ra
__sys_close:		# void __sys_close(unsigned int fd)
        li      $v0, _SYSCLOSEFILE
        syscall
        jr      $ra
__sys_read:		# int __sys_read(unsigned int fd, unsigned char *buf, unsigned int size);
        li      $v0, _SYSREADFILE
        syscall
        jr      $ra
__sys_write:		# int __sys_write(unsigned int fd, unsigned char *buf, unsigned int size);
        li      $v0, _SYSWRITEFILE
        syscall
        jr      $ra
__sys_sbrk:		# void* __sys_sbrk(unsigned int bytes);
        li      $v0, _SYSSBRK
        syscall
        jr      $ra
__sys_brk:		# void __sys_brk(void* addr);
        li      $v0, _SYSBRK
        syscall
        jr      $ra
