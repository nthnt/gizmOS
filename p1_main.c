//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//this file sets up the UART
#include "uart.h"

#include "_threadsCore.h"
#include "_kernelCore.h"
#include <math.h>

int x = 0;
int pins[8] = {6, 5, 4, 3, 2, 31, 29, 28};


void threadBTest (void* args)
{
	while(1)
	{
		//mutex acquire and release as soon as task is done and then cooperatively yield
		osAcquireMutex (0, OS_WAITFOREVER);
		printf("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\n");	
		osReleaseMutex (0);
		
		
		osYield();
	}
}

void threadATest (void* args)
{
	while(1)
	{
		//mutex acquire and release as soon as task is done and then cooperatively yield
	  osAcquireMutex (0, OS_WAITFOREVER);
		printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n");
		osReleaseMutex (0);
		
		osYield();
	}
}



void thread1 (void* args)
{
	while(1)
	{
		//mutex acquire and release as soon as task is done and then cooperatively yield
	  osAcquireMutex (0, OS_WAITFOREVER);
			printf("Thread 1\n");	
		osReleaseMutex (0);
		
		osYield();
	}
}

void thread2 (void* args)
{
	while(1)
	{
		//mutex acquire and release as soon as task is done and then cooperatively yield
	  osAcquireMutex (0, OS_WAITFOREVER);
			printf("Thread 2\n");	
		osReleaseMutex (0);
		
		osYield();
	}
}

void thread3 (void* args)
{
	while(1)
	{
		//mutex acquire and release as soon as task is done and then cooperatively yield
	  osAcquireMutex (0, OS_WAITFOREVER);
		printf("Thread 3\n");	
		osReleaseMutex (0);
		
		osYield();
	}
}



void threadIncrement (void* args)
{
	while(1)
	{
	  osAcquireMutex (0, OS_WAITFOREVER);
			x++;
			printf("x++: %d\n", x);
		osReleaseMutex (0);
		
		osYield();
	}
}

void threadLEDX (void* args)
{
	while(1)
	{
		int mod;
		
		osAcquireMutex (0, OS_WAITFOREVER);
				mod = x%47;
		osReleaseMutex (0);
		
		
		osAcquireMutex (1, OS_WAITFOREVER);
		//clear LEDs
		LPC_GPIO1->FIOCLR |= 0xBU<<28; //shift (1011) over 28
		LPC_GPIO2->FIOCLR |= 0x1FU<<2; //shift (00011111) over 2 
		
		//set the LEDs according to the binary number from x%47
		for (int d = 0; d < 5; d++) {
				if ((mod&((int) pow(2, d))) == ((int) pow(2, d))) {
						LPC_GPIO2->FIOSET |= 1<<pins[d];
				}
		}
		
		for (int d = 5; d < 8; d++) {
				if ((mod&((int) pow(2, d))) == ((int) pow(2, d))) {
						LPC_GPIO1->FIOSET |= 1<<pins[d];
				}
		}
		
		printf("LEDs > x%%47=%d\n", mod);
		osReleaseMutex (1);
		
		osYield();
	}
}

void threadLED (void* args)
{
	while(1)
	{
	  osAcquireMutex (1, OS_WAITFOREVER);
		printf("LEDs > 0x71\n");

		//set LEDs to 0x71
		LPC_GPIO1->FIOCLR |= 0xBU<<28; //shift (1011) over 28
		LPC_GPIO2->FIOCLR |= 0x1FU<<2; //shift (00011111) over 2 

		LPC_GPIO1->FIOSET |= 0<<28;
		LPC_GPIO1->FIOSET |= 1<<29;
		LPC_GPIO1->FIOSET |= 1<<31;
		LPC_GPIO2->FIOSET |= 1<<2;
		LPC_GPIO2->FIOSET |= 0<<3;
		LPC_GPIO2->FIOSET |= 0<<4;
		LPC_GPIO2->FIOSET |= 0<<5;
		LPC_GPIO2->FIOSET |= 1<<6;
		osReleaseMutex (1);
		
		osYield();
	}
}


int main( void ) 
{
	//Always call this function at the start. It sets up various peripherals, the clock etc. If you don't call this
	//you may see some weird behaviour
	SystemInit();
	
	//set directions to set or clear LEDs later
	LPC_GPIO1->FIODIR |= 0xBU<<28;
	LPC_GPIO2->FIODIR |= 0x1FU<<2;
	
	//run the core frequency of OS at around 1 interrupt/ms
	SysTick_Config(SystemCoreClock/1000);
	//initialize kernel
	kernelInit();
	
	printf("initialized\n");
	
	// PRELIMINARY TEST ============================
	/*
	newThread(threadBTest);
	newThread(threadATest);
	*/
	
	// TEST 1 ============================
	/*
	newThread(thread3);
	newThread(thread2);
	newThread(thread1);
	*/
	
	// TEST 2 ============================
	newThread(threadLEDX);
	newThread(threadIncrement);
	newThread(threadLED);
	
	
	//start kernel
	osKernelStart();
	
	//Your code should always terminate in an endless loop if it is done. If you don't
	//the processor will enter a hardfault and will be weird
	while(1);
}
