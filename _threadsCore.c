//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//include appropriate header files for forward declarationss
#include "_threadsCore.h"

//obtains initial MSP location via vector table at address 0x0
uint32_t* getMSPInitialLocation(void) {
	uint32_t* p = 0x0;
	return (uint32_t*)*p; //dereference p to get MSP location (as integer) at 0x0 then cast to pointer 
}

//creates new PSP that is offset bytes from MSP
uint32_t* getNewThreadStack(uint32_t offset){
	uint32_t msp_int = (uint32_t) getMSPInitialLocation(); //cast MSP location to integer to avoid pointer arithmetic
	uint32_t psp_int = msp_int - offset;
	
	printf("\n");
	
	//error checking and returns null if the offset is greater than size
	//of max stack (8192 bytes)
	if(msp_int - psp_int > 8192) {
		printf("Stack Size too large! \n");
		return NULL; 
	}
	
	return (uint32_t*) psp_int; //return pointer to new PSP
}

//sets the PSP value and control bit
void setThreadingWithPSP(uint32_t* threadStack) {
	 //sets PSP register with PSP passed as parameter
	__set_PSP((uint32_t) threadStack);
	//sets control bit to 2 to indicate PSP used
	__set_CONTROL(1 << 1); 
}
