#ifndef THREADSCORE_H
#define THREADSCORE_H

#include <stdint.h>

//Obtains the initial location of MSP by looking it up in the vector table
uint32_t* getMSPInitialLocation(void);
//Returns the address of a new PSP with offset of “offset” bytes from MSP. Be careful with
//pointer arithmetic! It’s best to cast to an integer then back if you’re not sure.
uint32_t* getNewThreadStack(uint32_t offset); 
//create new thread
void newThread(void (*threadFunc) (void* args), uint32_t deadline, uint32_t frequency);
//task switch
int task_switch(void);
#endif
