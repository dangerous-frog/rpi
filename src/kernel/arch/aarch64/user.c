#include "user_sys.h"
#include "printf.h"





void loop(char* str)
{
	char buf[2] = {""};
	call_sys_set_prio(100);
	int counter = 0;
	while (1){
		call_sys_write("\nISR Fired %d\n", counter);
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
		loop_constant("a");
	} else {
		loop("ISR Fired\n\n");
	}
	
}