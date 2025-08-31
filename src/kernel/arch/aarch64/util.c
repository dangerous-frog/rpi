void mmio_write(long reg, unsigned int val) { *(volatile unsigned int *)reg = val; }
unsigned int mmio_read(long reg) { return *(volatile unsigned int *)reg; }
void delay(volatile int ms) {
        // Yes, it's this stupid for now. lmao
        while (ms > 0) {ms--;}
}

void set_pgd(unsigned long pgd) {
        __asm__ volatile (
                "msr ttbr0_el1, %0\n\t"
                "tlbi vmalle1is\n\t" // We need to clear this buffer
                "dsb ish\n\t"
                "isb"
                :
                : "r" (pgd)
                : "memory"
        );
}

unsigned long get_pgd(void) {
        unsigned long result;
        __asm__ volatile (
                "mov x1, #0\n\t"
                "ldr %0, [x1]\n\t"
                "mov %0, #0x1000\n\t"
                "msr ttbr0_el1, %0\n\t"
                "ldr %0, [x1]"
                : "=r" (result)
                :
                : "x1", "memory"
        );
        return result;
}