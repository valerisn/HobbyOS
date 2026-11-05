#ifndef CPU_GDT_H
#define CPU_GDT_H

#define SEL_CODE 0x08
#define SEL_DATA 0x10
#define SEL_TSS 0x18

void gdt_init(void);

#endif
