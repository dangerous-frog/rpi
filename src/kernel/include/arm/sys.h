#ifndef SYS_H
#define SYS_H

#define __NR_syscalls 4

#define SYS_WRITE_NUMBER 0
#define SYS_MALLOC_NUMBER 1
#define SYS_CLONE_NUMBER 2
#define SYS_EXIT_NUMBER 3

#define ESR_ELx_EC_SHIFT		26
#define ESR_ELx_EC_SVC64		0x15
#define ESR_ELx_EC_DABT_LOW		0x24

#ifndef __ASSEMBLER__



void sys_write(char * buf);
int sys_fork();

void call_sys_write(char * buf);
int call_sys_clone(unsigned long fn, unsigned long arg, unsigned long stack);
unsigned long call_sys_malloc();
void call_sys_exit();

#endif

#endif


