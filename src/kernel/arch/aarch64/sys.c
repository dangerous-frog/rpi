#include "printf.h"
#include "sched.h"
#include "mm.h"
#include "printf.h"
#include "drivers/io.h"
#include "drivers/fb.h"



void sys_write(char * fmt, ...){
	// Yeah, bit goofy, but works and this is LEARNING OS
	va_list va;
	va_start(va, fmt);
	tfp_format(0, uart_writeChar, fmt, va);
	va_end(va);
}

int sys_fork(){
	return copy_process(0, 0, 0, 0);
}

void sys_exit(){
	exit_process();
}

void sys_delay_ticks(long ticks){
	delay_ticks(ticks);
}

char sys_uart_read_char(char * buf) {
	return uart_read_from_fifo(buf);
}

void sys_set_prio(int prio){
	set_task_prio(prio);
}


void sys_register_for_isr(int isr_num) {
	register_for_isr(isr_num);
}

void sys_write_screen_buffer(char *buf, int size_x, int size_y) {
    
    for (int x = 0; x < size_x; x++) {
        for (int y = 0; y < size_y; y++) {
            char c = buf[x * size_y + y];    
            drawChar(c, x * 8, y * 8, 0x0f);
        }
    }
}

void sys_clear_screen() {
	drawRect(0, 0, 1920, 1080, 0, 1);
}


void * const sys_call_table[] = {	sys_write, sys_fork, sys_exit, sys_delay_ticks, 
									sys_set_prio, sys_uart_read_char, sys_register_for_isr, sys_write_screen_buffer, sys_clear_screen};


