//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//this file sets up the UART
#include "uart.h"

#include "_threadsCore.h"
#include "_kernelCore.h"
extern threadNode* currentNode;

void periodic_12 (void* args) {
	while(1) {
		printf("periodic_12Hz\n");
	}
}
void periodic_100 (void* args) {
	while(1) {
		printf("periodic_100Hz\n");
	}
}
void periodic_256 (void* args) {
	while(1) {
		printf("periodic_256Hz\n");
	}
}

void yields (void* args)
{
	while(1)
	{
	  printf("yielding\n");	
		
		osYield();
	}
}
void periodic (void* args)
{
	while(1)
	{
		printf("periodic\n");
	}
}
void sleeps (void* args)
{
	while(1)
	{
		printf("SLEEPING*****************\n");
		
		osThreadSleep(20);
	}
}



void sleeps1 (void* args)
{
	while(1)
	{
		printf("SLEEPING1*****************\n");
		osThreadSleep(23);
	}
}
void sleeps2 (void* args)
{
	while(1)
	{
		printf("SLEEPING2*****************\n");
		osThreadSleep(17);
	}
}


int main( void ) 
{
	//Always call this function at the start. It sets up various peripherals, the clock etc. If you don't call this
	//you may see some weird behaviour
	SystemInit();
	
	//run the core frequency of OS at around 1 interrupt/ms
	SysTick_Config(SystemCoreClock/1000);
	//initialize kernel
	kernelInit();
	
	printf("initialized\n");
	
	// TEST 1 ============================
	/*
	newThread(yields, 7, NULL);
	newThread(periodic, 5, 200);
	newThread(sleeps, 11, NULL);
	*/
	
	// TEST 2 ============================
	/*
	newThread(periodic_12, 10, 12);
	newThread(periodic_100, 10, 100);
	newThread(periodic_256, 10, 256);
	*/
	
	// TEST 3 ============================
	newThread(sleeps2, 7, NULL);
	newThread(sleeps1, 6, NULL);

	//start kernel
	osKernelStart();
	
	//Your code should always terminate in an endless loop if it is done. If you don't
	//the processor will enter a hardfault and will be weird
	while(1);
}
