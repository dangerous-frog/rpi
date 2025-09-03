#include "mm.h"

#define PERIPHERAL_BASE (0xFE000000 + VA_START)

void uart_init();
void uart_writeText(char *buffer);
// pointer unused but added so it works with printf lib
void uart_writeChar(void* p, char character);
char uart_readChar();

void mmio_write(unsigned long reg, unsigned int val);
unsigned int mmio_read(unsigned long reg);