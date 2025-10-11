#include "user_sys.h"
#include "printf.h"
#include "term.h"





void loop(char* str)
{
    term_init(call_sys_clear_screen,call_sys_write_screen_buffer);
	term_printf("siema mordo");
	refresh_screen();
	char buf[2] = {""};
	call_sys_set_prio(100);
	int counter = 0;
	while (1){
		clear_screen();
		term_printf("\nISR Fired %d\n", counter);
		refresh_screen();
		counter++;
		call_sys_delay_ticks(8192);
	}
}

void loop_constant(char* str)
{
	char buf[2] = {""};
	while (1){
		for (int i = 0; i < 5; i++){
			buf[0] = str[i];
			call_sys_write(buf);
			user_delay(10000000);
		}
	}
}

void read_from_uart() {
	while (1) {
		char read[12]; 
		while (read != '\0') {
			int len = call_sys_uart_read_char(read);
			for (int i = 0; i < len; i++) {
				call_sys_write("%c", read[i]);
			}
		}
		
	}
}

void user_process() 
{
	call_sys_write("User process\n\r");
	int pid = call_sys_fork();
	if (pid < 0) {
		call_sys_write("Error during fork\n\r");
		call_sys_exit();
		return;
	}
	if (pid == 0){
		read_from_uart();
	} else {
		loop("ISR Fired\n\n");
	}
	
}