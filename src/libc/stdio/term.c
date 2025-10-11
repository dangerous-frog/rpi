#include "term.h"
#include "printf.h"

// 0 , 0 is top left corner
static char buffer[CHAR_X_LEN][CHAR_Y_LEN];

static int cursor = 0;

static void (*clear_screen_ptr)(void);
static void (*write_buffer_ptr)(char*, int , int );

void term_init(void (*clear_screen)(void), void (*write_buffer)(char* data, int size_x, int size_y)){
    clear_screen_ptr = clear_screen;
    write_buffer_ptr = write_buffer;
}

void refresh_screen() {
    buffer[0][0] = 'x';
    buffer[CHAR_X_LEN -1][CHAR_Y_LEN - 1] = 'x';
    clear_screen_ptr();
    write_buffer_ptr((char*)buffer, CHAR_X_LEN, CHAR_Y_LEN);
}

void write_char_to_buffer(void *p, char character) {
    int pos_x = cursor % CHAR_X_LEN;
    int pos_y = cursor / CHAR_X_LEN;
    buffer[pos_x][pos_y] = character;
    cursor++;
}

void term_printf(char * fmt, ...) {
    va_list va;
    va_start(va, fmt);
    tfp_format(0, write_char_to_buffer, fmt, va);
    va_end(va);
}

void clear_screen() {
    cursor = 0;
    for (int x = 0; x < CHAR_X_LEN; x++) {
        for (int y = 0; y < CHAR_Y_LEN; y++) {
            buffer[CHAR_X_LEN][CHAR_Y_LEN] = 0;
        }
    }
}