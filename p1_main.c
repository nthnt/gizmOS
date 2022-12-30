//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//this file sets up the UART
#include "uart.h"

#include "_threadsCore.h"
#include "_kernelCore.h"

void task1(void* args) {
	while(1) {
	  printf("this is task 1\n");
		osYield();
	}
}
void task2(void* args) {
	while(1) {
		printf("this is task 2\n");
		osYield();
	}
}

void idle(void* args) {
	while(1) {
		printf("idle thread\n");
		osYield();
	}
}

int main(void) {
	//Always call this function at the start. It sets up various peripherals, the clock etc. If you don't call this
	//you may see some weird behaviour
	SystemInit();
	
	//initialize kernel
	kernelInit();
	
	printf("initialized\n");
	
	//creating threads 
	newThread(idle);
	newThread(task1);
	newThread(task2);
	
	//start kernel
	osKernelStart();
	
	//Your code should always terminate in an endless loop if it is done. If you don't
	//the processor will enter a hardfault and will be weird
	while(1);
}
