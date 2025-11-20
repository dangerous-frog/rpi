#include "term.h"
#include "user_printf.h"
// 0 , 0 is top left corner
static char buffer[CHAR_X_LEN][CHAR_Y_LEN];
static uint8_t needs_update[CHAR_X_LEN][CHAR_Y_LEN];

static int cursor = 0;

static void (*clear_screen_ptr)(void);
static void (*write_buffer_ptr)(char*, uint8_t*, int , int );

void term_init(void (*clear_screen)(void), void (*write_buffer)(char* data, uint8_t* needs_update, int size_x, int size_y)){
    clear_screen_ptr = clear_screen;
    write_buffer_ptr = write_buffer;
    for(int i = 0; i < CHAR_X_LEN; i++) {
        for (int j = 0; j < CHAR_Y_LEN; j++) {
            buffer[i][j] = ' ';
        }
    }
}

void refresh_screen() {
    // clear_screen_ptr();
    write_buffer_ptr((char*)buffer, (uint8_t*)needs_update, CHAR_X_LEN, CHAR_Y_LEN);
}

void write_char_to_buffer(void *p, char character) {
    if (character < 26 || character > 126) 
        return;
    if (character == '\n')
        cursor += CHAR_X_LEN;
    int pos_x = cursor % CHAR_X_LEN;
    int pos_y = cursor / CHAR_X_LEN;
    buffer[pos_x][pos_y] = character;
    needs_update[pos_x][pos_y] = 1;
    cursor++;
}

void term_printf(char * fmt, ...) {
    va_list va;
    va_start(va, fmt);
    tfp_format_user(0, write_char_to_buffer, fmt, va);
    va_end(va);
}

void clear_screen() {
    cursor = 0;
    for (int x = 0; x < CHAR_X_LEN; x++) {
        for (int y = 0; y < CHAR_Y_LEN; y++) {
            if (buffer[x][y] != ' ') { 
                buffer[x][y] = ' ';
                needs_update[x][y] = 1;
            }
        }
    }
}
