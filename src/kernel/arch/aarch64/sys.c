#include "printf.h"
#include "sched.h"
#include "mm.h"
#include "printf.h"
#include "drivers/io.h"



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

void sys_set_prio(int prio){
	set_task_prio(prio);
}

void * const sys_call_table[] = {sys_write, sys_fork, sys_exit, sys_delay_ticks, sys_set_prio};


