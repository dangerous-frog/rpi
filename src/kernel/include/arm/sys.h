#ifndef SYS_H
#define SYS_H

#define __NR_syscalls 8


#define ESR_ELx_EC_SHIFT		26
#define ESR_ELx_EC_SVC64		0x15
#define ESR_ELx_EC_DABT_LOW		0x24

#ifndef __ASSEMBLER__
#include <stdarg.h>



void sys_write(char * buf);
int sys_fork();
void call_sys_write(char * fmt, ...);
void call_sys_write_char(void *p, char character);
char call_sys_uart_read_char(char * buf);
int call_sys_clone(unsigned long fn, unsigned long arg, unsigned long stack);
void call_register_for_isr(int isr_num);
unsigned long call_sys_malloc();
void call_sys_exit();

#endif

#endif


