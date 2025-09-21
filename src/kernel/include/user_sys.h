#ifndef	_USER_SYS_H
#define	_USER_SYS_H
#include <stdarg.h>




// Usage:


void call_sys_write(char * fmt, ...);
int call_sys_fork();
void call_sys_exit();
void call_sys_delay_ticks(long ticks);
void call_sys_set_prio(int prio);
void call_sys_write_char(void *p, char character);

extern void user_delay ( unsigned long);
extern unsigned long get_sp ( void );
extern unsigned long get_pc ( void );

#endif  /*_USER_SYS_H */