#ifndef	_P_IRQ_H
#define	_P_IRQ_H

#define PBASE 0xFF800000 // taken from manual interrupts ch.

#define PERI_IRQ_ROUTE0     (PBASE + 0x24)
#define LOCAL_TIMER_CONTROL (PBASE + 0x34)
#define LOCAL_TIMER_IRQ     (PBASE + 0x38)
#define TIMER_CNTRL0        (PBASE + 0x40)
#define IRQ_SOURCE0         (PBASE + 0x60)

#define CLOCK_TICKS         20000 // 250 000 000, should fit

// Below we have the registers for the GIC
#define GIC_BASE 0xFF840000

// According to manual we have 2 important ares
// 1. Distributor at + 0x1000
// 2. Cpu interfaces at 0x2000
// Both need setting to enable interrups and routing them to proper core
#define GICD                (GIC_BASE + 0x1000)
#define GICC                (GIC_BASE + 0x2000)

#define GICD_ISENABLERn     (GICD + 0x100) //enable
#define GICD_ITARGETSRn     (GICD + 0x800) // route to cpu
#define GICD_IPRIORITYRn    (GICD + 0x400)
#define GICD_ISPENDRn       (GICD + 0x200)


#define GICC_IAR            (GICC + 0x0c) // ack interrupt
#define GICC_EOIR           (GICC + 0x10) // end of interrupt register?? not sure what this means lol
#define GICC_PMR            (GICC + 0x0004)
#define GICC_HPPIR          (GICC + 0x0018)

void enable_interrupt(int irq_num);



#endif  /*_P_IRQ_H */