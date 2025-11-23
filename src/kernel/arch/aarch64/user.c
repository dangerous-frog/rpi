#include "user_sys.h"
#include "printf.h"
#include "term.h"




void loop()
{
	call_sys_set_prio(100);

	while (1){
		refresh_screen();	
		call_sys_delay_ticks(128);
	}
}

void get_command(char* command) {
	int cmd_cursor = 0;
	char read[12];

	while(1) {
		// Get some chars and process them
		int len = call_sys_uart_read_char(read);
		for(int i = 0; i < len; i++) {
			switch (read[i])
			{
			case 127:
				term_printf("%c", 127);
				command[cmd_cursor--] = '\n';
				break;

			case '\r':
				// We have the whole command
				return;
			default:
				command[cmd_cursor++] = read[i];
				term_printf("%c",read[i]);
				break;
			}
		}
	}
}


void terminal() {
	
	while(1) {
		char command[64] = "";

		term_printf("TERM>");
		
		get_command(command);

		call_sys_write("Processing %s\n", command);

		// process command

		term_printf("\r");
	}
}




void user_process() 
{
	call_sys_write("First user process\n\r");
	term_init(&call_sys_clear_screen, &call_sys_write_screen_buffer);
	int pid = call_sys_fork();
	if (pid < 0) {
		call_sys_write("Error during fork\n\r");
		call_sys_exit();
		return;
	}
	if (pid == 0)
		terminal();
	loop();
	
}