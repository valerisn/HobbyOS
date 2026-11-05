#ifndef CPU_IDT_H
#define CPU_IDT_H

void idt_init(void);
/* irq is the pic line (0 to 15), the handler runs with interrupts off */
void irq_register(int irq, void (*fn)(void));

#endif
