#ifndef	_UTILS_H
#define	_UTILS_H

extern int get_el (void );
void mmio_write(long reg, unsigned int val);
unsigned int mmio_read(long reg);
extern void enable_irq( void);

#endif  /*_UTILS_H */