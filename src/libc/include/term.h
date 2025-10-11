#include <stdarg.h>


#define CHAR_X_LEN (1920/8)
#define CHAR_Y_LEN (1080/8)



void term_printf(char * fmt, ...);
void term_init(void (*clear_screen) (void), void (*write_buffer)(char *, int, int ));
void refresh_screen();
void clear_screen();