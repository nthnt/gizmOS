#include "_kernelCore.h"

void kernelInit(void) {
	//set priority of PendSV via bits 23-26 at defined memory address
	SHPR3 |= 0xFF << 16;
}

void osSched(void) {
	//set state to pending via bit 28 at defined memory address
	ICSR |= 1<<28; 
	//flush pipeline
	__asm("isb"); 
}
