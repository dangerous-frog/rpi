#include "drivers/io.h"
#include "drivers/fb.h"
#include "printf.h"
#include "arm/util.h"
#include "arm/irq.h"
#include "sched.h"
#include "arm/sys.h"
#include "user.h"
#include "heap.h"



extern void irq_vector_init( void );

// void user_process1(char *array)
// {
// 	char buf[2] = {0};
// 	while (1){
// 		for (int i = 0; i < 5; i++){
// 			buf[0] = array[i];
// 			call_sys_write(buf);
// 			delay(100000);
// 		}
// 	}
// }

// void user_process(){
// 	char buf[30] = {0};
// 	tfp_sprintf(buf, "User process started\n\r");
// 	call_sys_write(buf);
// 	unsigned long stack = call_sys_malloc();
// 	if (stack < 0) {
// 		printf("Error while allocating stack for process 1\n\r");
// 		return;
// 	}
// 	int err = call_sys_clone((unsigned long)&user_process1, (unsigned long)"12345", stack);
// 	if (err < 0){
// 		printf("Error while clonning process 1\n\r");
// 		return;
// 	} 
// 	stack = call_sys_malloc();
// 	if (stack < 0) {
// 		printf("Error while allocating stack for process 1\n\r");
// 		return;
// 	}
// 	err = call_sys_clone((unsigned long)&user_process1, (unsigned long)"abcd", stack);
// 	if (err < 0){
// 		printf("Error while clonning process 2\n\r");
// 		return;
// 	} 
// 	call_sys_exit();
// }

void kernel_process(){
	printf("Kernel process started. EL %d\r\n", get_el());
	unsigned long begin = (unsigned long)&user_begin;
    unsigned long end = (unsigned long)&user_end;
    unsigned long process = (unsigned long)&user_process;
	int err = move_to_user_mode(begin, end - begin, process - begin);
	if (err < 0){
		printf("Error while moving process to user mode\n\r");
	} 
}


void kernel_main()
{
    uart_init();
    fb_init();

    init_printf(0, uart_writeChar);    
    heap_init();
    sched_init();

    // Set up timer interrupt
    irq_vector_init();
    enable_irq();
    enable_interrupt(30);
    drawString(100,100,"Hello world!",0x0f);
    

    int res = copy_process(PF_KTHREAD, (unsigned long)&kernel_process, 0, 0);
	if (res < 0) {
		printf("error while starting kernel process");
		return;
	}


	while (1){
		schedule();
	}	


    // while (1) {
    //     // Here we will do uart reading and writing lol
    //     // Too poor for interrupts just now

    //     char read = uart_readChar();
    //     if (read != '\0') {
    //         printf("%c", read);
    //     } 


    // }
}
