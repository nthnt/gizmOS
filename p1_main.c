//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//this file sets up the UART
#include "uart.h"

#include "_threadsCore.h"
#include "_kernelCore.h"
extern threadNode* currentNode;

/*
void yields (void* args)
{
	while(1)
	{
	  printf("yielding: %x\n",__get_PSP());	
		
		osYield();
	}
}
void loops (void* args)
{
	while(1)
	{
		printf("looping: %x\n",__get_PSP());
	}
}
void sleeps (void* args)
{
	while(1)
	{
		printf("SLEEPING*****************: %x\n",__get_PSP());
		
		osThreadSleep(20);
	}
}
void idle (void* args)
{
	while(1)
	{
		printf("idle thread %x\n",__get_PSP());
		osYield();
	}
}
*/

void sleeps1 (void* args)
{
	while(1)
	{
		printf("SLEEPING1*****************: %x\n",__get_PSP());
		
		osThreadSleep(13);
	}
}
void sleeps2 (void* args)
{
	while(1)
	{
		printf("SLEEPING2*****************: %x\n",__get_PSP());
		
		osThreadSleep(23);
	}
}
void idle (void* args)
{
	while(1)
	{
		printf("idle thread %x\n",__get_PSP());
		osYield();
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
	
	/*
	//creating threads 
	newThread(yields);
	newThread(loops);
	newThread(sleeps);
	newThread(idle);
	*/
	
	newThread(sleeps2);
	newThread(sleeps1);
	newThread(idle);
	
	/*
	printf("T1: %p,%d\n",currentNode->threadStackP,((uint32_t)currentNode->threadStackP)%8);
	printf("T1: %p,%d\n",currentNode->next->threadStackP,((uint32_t)currentNode->next->threadStackP)%8);
	printf("T1: %p,%d\n",currentNode->next->next->threadStackP,((uint32_t)currentNode->next->next->threadStackP)%8);
		
		*/
		//the processor will enter a hardfault and will be weird
	//while(1);
	//start kernel
	osKernelStart();
	
	//Your code should always terminate in an endless loop if it is done. If you don't
	//the processor will enter a hardfault and will be weird
	while(1);
}
