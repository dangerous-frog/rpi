void mmio_write(long reg, unsigned int val) { *(volatile unsigned int *)reg = val; }
unsigned int mmio_read(long reg) { return *(volatile unsigned int *)reg; }
void delay(volatile int ms) {
        // Yes, it's this stupid for now. lmao
        while (ms > 0) {ms--;}
}