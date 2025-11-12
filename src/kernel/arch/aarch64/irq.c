#include "printf.h"
#include "arm/entry.h"
#include "arm/irq.h"
#include "drivers/io.h"
#include "arm/util.h"
#include "sched.h"
#include "stddef.h"
#include "stdint.h"

const char *entry_error_messages[] = {
	"SYNC_INVALID_EL1t",
	"IRQ_INVALID_EL1t",		
	"FIQ_INVALID_EL1t",		
	"ERROR_INVALID_EL1T",		

	"SYNC_INVALID_EL1h",		
	"IRQ_INVALID_EL1h",		
	"FIQ_INVALID_EL1h",		
	"ERROR_INVALID_EL1h",		

	"SYNC_INVALID_EL0_64",		
	"IRQ_INVALID_EL0_64",		
	"FIQ_INVALID_EL0_64",		
	"ERROR_INVALID_EL0_64",	

	"SYNC_INVALID_EL0_32",		
	"IRQ_INVALID_EL0_32",		
	"FIQ_INVALID_EL0_32",		
	"ERROR_INVALID_EL0_32"	
};

static uint64_t timer_freq;



void load_timer_interrupt() {
	uint64_t count;
	asm volatile("mrs %0, cntpct_el0" : "=r"(count));
	// timer_freq is 1 sec
	count += timer_freq / 128;  // 0.0004 //TODO: see if it does it
	asm volatile("msr cntp_cval_el0, %0" : : "r"(count));
}
// TODO: this should include core, for now only 0
void enable_interrupt(int irq_num)
{	

	// For any changes to take effect, we need to disable/enable gic
	mmio_write(GICD, 0);
        
    // Set target CPU (assuming CPU0)
    unsigned long reg = irq_num / 4;
    int shift = (irq_num % 4) * 8;
    mmio_write(GICD_ITARGETSRn + reg*4, 1 << shift);
    
    // Set priority
    reg = irq_num / 4;
    shift = (irq_num % 4) * 8;
    mmio_write(GICD_IPRIORITYRn + reg*4, 0x90 << shift);
    
    // Enable interrupt
    reg = irq_num / 32;
    shift = irq_num % 32;
    mmio_write(GICD_ISENABLERn + reg*4, 1 << shift);
    
    // 3. Enable distributor
    mmio_write(GICD, 1);
    
    // 4. CPU interface
    mmio_write(GICC_PMR, 0xFF);  // Allow all priorities
    mmio_write(GICC, 1);    // Enable CPU interface

	// This code was for turning on external interrupt
	// Set up what timer we want
	// Route that interrupt as normal IRQ not FIQ
	// mmio_write(TIMER_CNTRL0, (1 << 1)); 
	// Route that interrupt to our cpu
	// mmio_write(PERI_IRQ_ROUTE0, (0x01 << 24) | (0));
	

	// Let's try the local timer
	// while ( 1) {
	// 	unsigned int irq = mmio_read(IRQ_SOURCE0) & (1 << 11);
	// 	if (irq) {	
	// 		printf("Interrupt fired\n");
	// 		mmio_write(LOCAL_TIMER_IRQ, (1 << 31));

	// 	} else {
	// 		printf("No interrupt\n");
	// 	}
	// }
}

void enable_timer() {
	// Get timer frequency
    asm volatile("mrs %0, cntfrq_el0" : "=r"(timer_freq));
	printf("Freq %d\n",timer_freq );

	load_timer_interrupt();	
    
    // Enable timer with interrupt
    asm volatile("msr cntp_ctl_el0, %0" : : "r"(1));
}

void show_invalid_entry_message(int type, unsigned long esr, unsigned long address)
{
	printf("Type: %d, %s, ESR: %x, address: %x\r\n",type, entry_error_messages[type], esr, address);
}

void handle_irq(void) {
    uint32_t irq_id = mmio_read(GICC_IAR) & 0x3FF;
    // printf("\nIRQ %d\n", irq_id);
	if ( irq_id < NR_ISR) {
		// Let scheduling know

		handle_isr_wake_up(irq_id);
	}
	

	switch (irq_id) {
		case 30:
			// Set next timer interrupt
			// Because timer_tick will likely switch context, it's very important that we first process/restart the interrupt
			mmio_write(GICC_EOIR, irq_id);
			load_timer_interrupt();
			timer_tick();
			break;
			
		case 125:
			// Uart read isr
			// first check if it is uart read isr, as it's OR'ed isr
			uint32_t aux_irq = mmio_read(AUX_IRQ) & 0x01;
			if (!aux_irq) {
				printf("Unknown AUX IRQ");
				break;
			}
			mmio_write(GICC_EOIR, irq_id);
			uart_readChar();
			break;
		case 1023:
			printf("Spurious interrupt\n");
			break;
			
		default:
			printf("Unexpected interrupt: %d\n", irq_id);
			mmio_write(GICC_EOIR, irq_id);
			break;
	}
	// printf("\nexiting handler\n");
}