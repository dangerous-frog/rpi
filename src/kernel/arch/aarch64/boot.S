#include "arm/sysregs.h"
#include "mm.h"
#include "arm/mmu.h"

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

.macro print_hex, reg
    mov     x0, \reg
    mov     x1, #0x3C000000     // QEMU debug console address
    str     x0, [x1]
.endm

el1_entry:
    // Seems el1 doesn't have fp enabled by default
    // we need to enable it or it dies on first calculation in printf lmao 
    // scouring the manual just yields that yes this needs to be enabled
    // but I feel it needs something deeper
    mrs x0, cpacr_el1
    orr x0, x0, #(3 << 20)    // Enable FP/SIMD at EL1
    msr cpacr_el1, x0
    
    isb
    

    // Clean the BSS section
    adr     x0, bss_begin     // Start address
    adrp     x1, bss_end      // Size of the section
    add     x1, x1, :lo12:bss_end
    sub     x1, x1, x0
    bl      memzero


    bl      create_page_tables

    // Since we just actually started using virt mem
    // then the stack pointer gotta use it too
    mov     x0, #VA_START
    add     sp, x0, #LOW_MEMORY

    // Tell mmu where our PGD is
    adrp    x0, pg_dir
    msr     ttbr1_el1, x0

    print_hex x0

    adrp    x0, pg_dir_identity
    msr     ttbr0_el1, x0 // for identity mapping

    // Configure MMU, 4kb pages
    ldr	x0, =(TCR_VALUE)		
	msr	tcr_el1, x0

    // Set mair values
	ldr	x0, =(MAIR_VALUE)
	msr	mair_el1, x0

    // After turning on MMU, we can't just branch
    // It would hold old address
    // so we use the one from linker
    ldr	x2, =kernel_main

	mov	x0, #SCTLR_MMU_ENABLED				
	msr	sctlr_el1, x0 // Enable mmu

	br 	x2


    b   1b // Loop forever just in case

    //                          0   1       2                 3   4
    .macro	create_pgd_entry, tbl, virt, target_table_phys, tmp1, tmp2
    mov     \tmp1, #1
    lsl     \tmp1, \tmp1, #PUD_TARGET_OFFSET 
    add	    \target_table_phys, \tbl, #PAGE_SIZE                 // We know 1st PMD is after PGD   
	create_table_entry \tbl, \virt, PGD_SHIFT, \tmp2, \target_table_phys
    add	    \tbl, \tbl, #PAGE_SIZE					// next level table page
    add     \target_table_phys, \target_table_phys, #PAGE_SIZE 
	create_table_entry \tbl, \virt, PUD_SHIFT, \tmp2, \target_table_phys // 1st PMD
    add     \virt, \virt, \tmp1
    add     \target_table_phys, \target_table_phys, #PAGE_SIZE                 // Next one follows it and so on....
    create_table_entry \tbl, \virt, PUD_SHIFT, \tmp2, \target_table_phys // 2nd PMD
    add     \virt, \virt, \tmp1
    add     \target_table_phys, \target_table_phys, #PAGE_SIZE
    create_table_entry \tbl, \virt, PUD_SHIFT, \tmp2, \target_table_phys // 3rd PMD
    add     \virt, \virt, \tmp1
    add     \target_table_phys, \target_table_phys, #PAGE_SIZE             
    create_table_entry \tbl, \virt, PUD_SHIFT, \tmp2, \target_table_phys // 4th PMD
    add     \tbl, \tbl, #PAGE_SIZE
	.endm

    /*
        tbl -> pointer to region where we gonna plop it
        virt -> vaddr which we're mapping rn
        shift -> shift needed to get table index
        tmp1, tmp2 -> as name suggests, temp regs
    */
    .macro	create_table_entry, tbl, virt, shift, tmp1, target_table_phys
	lsr	    \tmp1, \virt, #\shift                       // Shift to extract index bits
	and	    \tmp1, \tmp1, #PTRS_PER_TABLE - 1	// Mask to get index
	orr	    \target_table_phys, \target_table_phys, #MM_TYPE_PAGE_TABLE	// Setting these bits means it's table
    // Also we're modifying \target_table but it's only lower bits, should not be an issue
	// We store our entry (contents of tmp2) into tbl but index (tmp1) * 8 (left shift)
    // we do it times 8 since it is byte aligned and each entry is 8 bytes
    str	    \target_table_phys, [\tbl, \tmp1, lsl #3]
	.endm

    /*
        tbl -> pointer to PMD table
        phys -> actual address to be mapped
        start -> virt address of section to be mapped
        end -> virt address of the last section to be mapped
        flags -> flags that need to be copied into block descriptor
        tmp1 -> tmp reg
     */
    .macro	create_block_map, tbl, phys, start, end, flags, tmp1
	lsr	\start, \start, #SECTION_SHIFT
	and	\start, \start, #PTRS_PER_TABLE - 1			// table index
	lsr	\end, \end, #SECTION_SHIFT
	and	\end, \end, #PTRS_PER_TABLE - 1				// table end index
    // Now both start and end hold indexes corresponding to og addresses
	lsr	\phys, \phys, #SECTION_SHIFT                // Clear first part of addr
	mov	\tmp1, #\flags
	orr	\phys, \tmp1, \phys, lsl #SECTION_SHIFT			// table entry content
9999:	str	\phys, [\tbl, \start, lsl #3]				// store the entry
	add	\start, \start, #1					// next entry
	add	\phys, \phys, #SECTION_SIZE				// next block
	cmp	\start, \end
	b.ls	9999b
	.endm

create_page_tables:
    mov     x29, x30 // Save link register to x29

    // Zero the area where we'll have our table
    adrp    x0, pg_dir
    mov     x1, #PG_DIR_SIZE
    bl      memzero
    adrp    x0, pg_dir_identity
    mov     x1, #PG_DIR_SIZE
    bl      memzero


    // Create identity mapping for initial code
    adrp    x0, pg_dir_identity
    mov     x1, #0
    create_pgd_entry    x0, x1, x2, x3, x4

    // Nothing special here since it just fills 1st PMD
    mov     x1, #0
    mov     x2, #0
    mov     x3, #0x200000
    create_block_map x0, x1, x2, x3, MMU_FLAGS, x4


    // Here we create the table entries for PGD and PUD
    adrp    x0, pg_dir
    mov     x1, #VA_START
    create_pgd_entry x0, x1, x2, x3, x4

    /* Mapping kernel and init stack*/
    // This also just fills 1st PMD
	mov 	x1, xzr							// start mapping from physical offset 0
	mov 	x2, #VA_START						// first virtual address
	ldr	    x3, =(VA_START + HIGH_MEMORY - SECTION_SIZE)		// last virtual address
	create_block_map x0, x1, x2, x3, MMU_FLAGS, x4

	/* Mapping device memory*/
    adrp    x0, pg_dir
    add     x0, x0, #(PAGE_SIZE * 5) // We map devices to last table
	mov 	x1, #DEVICE_BASE					// start mapping from device base address 
	ldr 	x2, =(VA_START + DEVICE_BASE)				// first virtual address
	ldr	    x3, =(VA_START + DEVICE_END - SECTION_SIZE)	// last virtual address
	create_block_map x0, x1, x2, x3, MMU_DEVICE_FLAGS, x4

	mov	x30, x29						// restore return address
	ret



.global get_el
get_el:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    ret