.section ".text.user"



.macro SYSCALL_WRAPPER name, syscall_num
.globl call_sys_\name
call_sys_\name:
    mov w8, #\syscall_num
    svc #0
    ret
.endm


.globl user_delay
user_delay:
	subs x0, x0, #1
	bne user_delay
	ret


SYSCALL_WRAPPER write, 0
SYSCALL_WRAPPER fork, 1
SYSCALL_WRAPPER exit, 2
SYSCALL_WRAPPER delay_ticks, 3
SYSCALL_WRAPPER set_prio, 4