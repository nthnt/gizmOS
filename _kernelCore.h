#ifndef KERNELCORE_H
#define KERNELCORE_H
//header guard to avoid repeated declarations

#include <stdint.h>

//access the memory address that allows us to change the priority of PENDSV interrupt
#define SHPR3 *(uint32_t*)0xE000ED20 
//access the memory address that allows us to change PendSV function (such as pending status)
#define ICSR *(uint32_t*)0xE000ED04 

// initializes memory structures and interrupts necessary to run the kernel
void kernelInit(void);
// called by the kernel to schedule which threads to run
void osSched(void); 

#endif
