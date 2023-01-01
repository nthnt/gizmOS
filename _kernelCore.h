#ifndef KERNELCORE_H
#define KERNELCORE_H

#include <stdint.h>
#include <LPC17xx.h>
#include "osDefs.h"
#include "_threadscore.h"

#define SHPR3 *(uint32_t*)0xE000ED20
#define SHPR2 *(uint32_t*)0xE000ED1C
#define ICSR *(uint32_t*)0xE000ED04

// initializes memory structures and interrupts necessary to run the kernel
void kernelInit(void);
// called by the kernel to schedule which threads to run
void osYield(void); 
// sets the value of PSP to threadStack and ensures that the microcontroller is using that value by changing the CONTROL register
void setThreadingWithPSP(uint32_t* threadStack);
//start kernel
void osKernelStart(void);
//round robin scheduler
threadNode* scheduler(void);
//idle thread
void idle (void* args);
//makes the thread sleep
void osThreadSleep(uint32_t sleeptime);
//makes a mutex
uint32_t osMutexCreate(void);
//acquires a mutex
uint32_t osAcquireMutex (uint32_t mutexID, uint32_t timeout);
//releases a mutex
uint32_t osReleaseMutex (uint32_t mutexID);
#endif
