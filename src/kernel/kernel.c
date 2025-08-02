#include "io.h"
#include "fb.h"
// TODO: move those refs from here so they aren't in include since they aren't arch specific

void main()
{
    uart_init();
    fb_init();

    char buffer[2] = {'\0', '\0'};
    
    uart_writeText("Hello world!\n");
    drawString(100,100,"Hello world!",0x0f);
    while (1) {
        // Here we will do uart reading and writing lol
        // Too poor for interrupts just now

        buffer[0] = uart_readChar();
        if (buffer[0] != '\0') {
            uart_writeText(buffer);
        }


    }
}
