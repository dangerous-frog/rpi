#include "drivers/io.h"
#include "drivers/fb.h"
#include "printf.h"
#include "arm/util.h"
#include "arm/irq.h"
#include "sched.h"
// TODO: move those refs from here so they aren't in include since they aren't arch specific


extern void irq_vector_init( void );

void process(char *array)
{
	while (1){
		for (int i = 0; i < 5; i++){
			uart_writeChar(0, array[i]);
			delay(10000000);
		}
	}
}


void main()
{
    uart_init();
    fb_init();

    init_printf(0, uart_writeChar);

    
    
    
    uart_writeText("Hello world!\n");
    int el = get_el();
    
    printf("Exception level: %d \r\n", el);

    // Set up timer interrupt
    irq_vector_init();
    enable_irq();
    enable_interrupt(30);


    drawString(100,100,"Hello world!",0x0f);
    int res = copy_process((unsigned long)&process, (unsigned long)"12345");
	if (res != 0) {
		printf("error while starting process 1");
		return;
	}
	res = copy_process((unsigned long)&process, (unsigned long)"abcde");
	if (res != 0) {
		printf("error while starting process 2");
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
