#ifndef	_USER_SYS_H
#define	_USER_SYS_H
#include <stdarg.h>
#include "user_printf.h"




// Usage:



void call_sys_write(char * fmt, ...);
void call_sys_write_str(char * buffer);

void call_sys_write(char * fmt, ...) {
    char buf[256];
    
    va_list va;
    va_start(va, fmt);
    tfp_sprintf_u(buf, fmt, va);  // Format into buffer
    va_end(va);
    call_sys_write_str(buf);  // Syscall with formatted string
}
int call_sys_fork();
char call_sys_uart_read_char();
void call_sys_exit();
void call_sys_delay_ticks(long ticks);
void call_sys_set_prio(int prio);
void call_sys_write_char(void *p, char character);
void call_sys_register_for_isr(int isr_num);
void call_sys_write_screen_buffer(char * buffer, int size_x, int size_y);
void call_sys_clear_screen();

extern void user_delay ( unsigned long);
extern unsigned long get_sp ( void );
extern unsigned long get_pc ( void );

#endif  /*_USER_SYS_H */