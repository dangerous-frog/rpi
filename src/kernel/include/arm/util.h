#ifndef	_UTILS_H
#define	_UTILS_H

extern int get_el (void );
void mmio_write(unsigned long reg, unsigned int val);
unsigned int mmio_read(unsigned long reg);
extern void enable_irq( void);
extern void disable_irq(void);
void delay(int ms);
extern void set_pgd(unsigned long pgd);
extern unsigned long get_pgd();

#endif  /*_UTILS_H */