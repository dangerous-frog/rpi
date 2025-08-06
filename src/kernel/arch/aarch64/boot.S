#include "arm/sysregs.h"

.section ".text.boot"  // Make sure the linker puts this at the start of the kernel image

.global _start  // Execution starts here

_start:
    // Check processor ID is zero (executing on main core), else hang
    mrs     x1, mpidr_el1
    and     x1, x1, #3
    cbz     x1, 2f // f means forward, b would be backward
    // We're not on the main core, so hang in an infinite wait loop
1:  wfe
    b       1b
2:  // We're on the main core!

    ldr    x0, =SCTLR_VALUE_MMU_DISABLED
    msr    sctlr_el1, x0      
    

    ldr	    x0, =HCR_VALUE
	msr	    hcr_el2, x0

    ldr    x0, =SPSR_VALUE
    msr    spsr_el2, x0

    mov     x0, #0x3
    msr     cnthctl_el2, x0

    // set return address from el2
    adr     x0, el1_entry
    msr     elr_el2, x0

    eret // exception return


el1_entry:
    // Seems el1 doesn't have fp enabled by default
    // we need to enable it or it dies on first calculation in printf lmao 
    // scouring the manual just yields that yes this needs to be enabled
    // but I feel it needs something deeper
    mrs x0, cpacr_el1
    orr x0, x0, #(3 << 20)    // Enable FP/SIMD at EL1
    msr cpacr_el1, x0
    isb


    // Set stack to start below our code
    ldr     x1, =0x400000
    mov     sp, x1

    // Clean the BSS section
    ldr     x1, =__bss_start     // Start address
    ldr     w2, =__bss_size      // Size of the section
3:  cbz     w2, 4f               // Quit loop if zero
    str     xzr, [x1], #8
    sub     w2, w2, #8
    cbnz    w2, 3b               // Loop if non-zero

    // Jump to our main() routine in C (make sure it doesn't return)
4:  bl      main
    // In case it does return, halt the master core too
    b       1b

.global get_el
get_el:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    ret