//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//this file sets up the UART
#include "uart.h"

//include appropriate header files for thread and kernel functions
#include "_threadsCore.h"
#include "_kernelCore.h"


int main( void ) 
{
	//create a new PSP that is 512 bytes from MSP
	uint32_t* q = getNewThreadStack(512);

	//set the value of PSP to the created thread stack in previous step
	setThreadingWithPSP(q);
	
	printf("initial MSP address: %p \n", getMSPInitialLocation());
	printf("generated (from fn. 2) PSP address: %u \n", (uint32_t) q);
	printf("PSP address: %u \n", __get_PSP());
	printf("control bit: %u \n\n", __get_CONTROL());
	
	//Always call this function at the start. It sets up various peripherals, the clock etc. If you don't call this
	//you may see some weird behaviour
	SystemInit();
	
	//initialize the memory structures and interrupts necessary to run kernelInit
	kernelInit();
	//function that chedules which threads to run
	osSched();
	
	//Your code should always terminate in an endless loop if it is done. If you don't
	//the processor will enter a hardfault and will be weird
	while(1);
}
