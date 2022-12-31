#include "_kernelCore.h"

//for malloc
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

//int tasknum = 0;
bool startup = false;

//mutex
bool flag = true;

threadLinkedList* list;

//pointer to the currently running thread
threadNode* currentNode;

//sets the PSP value and control bit
void setThreadingWithPSP(uint32_t* threadStack) {
	//sets PSP register with PSP passed as parameter
	__set_PSP((uint32_t) threadStack);
	//sets control bit to 2 to indicate PSP used
	__set_CONTROL(1 << 1);
}


void kernelInit(void) {
	//allocate memory for linked list
	list = malloc(sizeof(threadLinkedList));
	list->head = NULL;
	list->size = 0;
	
	//set priority of PendSV via bits 23-26 at defined memory address
	SHPR3 |= 0xFF << 16;
}

void osYield(void) {
	flag = false;

	//reset timer
	currentNode->runTimer = OS_RUNTIME;
	printf(" \n");
	
	//set the current status to active and make space for its registers relative to current PSP
	if (!startup) {
		if (currentNode->status != SLEEPING){
			currentNode->status = ACTIVE;
		}
	  currentNode->threadStackP = (uint32_t*)(__get_PSP() - 16*4); //we are about to push 16 uint32_t's
		
		//scheduler decides what thread to run next
		scheduler();
	}
	else {
		startup = false;
	}

	//set the current node's status to be active and running
	currentNode->status = RUNNING;

	
	flag = true;
	
	//set state to pending via bit 28 at defined memory address
	ICSR |= 1<<28;
	//flush pipeline
	__asm("isb");
	//printf("hi");
}

void osThreadSleep(uint32_t sleeptime) {
	//sets the current node status to sleeping
	currentNode->status = SLEEPING;
	//sets the node's sleep timer
	currentNode->sleepTimer = sleeptime;
	//switch to the next thread
	osYield();
}

void SysTick_Handler(void) {
	if (!flag) {
		return;
	}
	//decrement the thread's run timer
	currentNode->runTimer--;
	
	//decrement all the sleep timers
	threadNode* dummyNode = currentNode;
	for (int x = 0; x < list->size; x++) {
		dummyNode = dummyNode->next;
		
		// if the current node is sleeping
		if (dummyNode->status == SLEEPING){
			dummyNode->sleepTimer--;

			//if sleeping timer is up, wake the thread
			if (dummyNode->sleepTimer == 0) {
				dummyNode->status = ACTIVE;
			}
		}
	}
	
	//if the thread is out of time
	if (currentNode->runTimer <= 0) {
		//reset the run timer
		currentNode->runTimer = OS_RUNTIME;
		//set the current status to active and make space for its registers relative to current PSP
		currentNode->status = ACTIVE;
		currentNode->threadStackP = (uint32_t*)(__get_PSP() - 8*4); //we are about to push 16 uint32_t's
		
		scheduler();
		
		//set the current node's status to be active and running
		currentNode->status = RUNNING;
		
		//set state to pending via bit 28 at defined memory address
		ICSR |= 1<<28;
		//flush pipeline
		__asm("isb");
	}
}

void scheduler (void) {
	currentNode = currentNode->next;
	
	// if the node the scheduler moved to is sleeping, we skip it
	while (currentNode->status == SLEEPING){
		currentNode = currentNode->next;
	}
}

void osKernelStart(void) {
	//check if any threads created and if so, set the first thread to run to avoid bootstrapping
	if(list->size > 0) {
		//set to -1 to instantly start running
		startup = true;		
		
		// setup currentNode, on startup, currentNode is set to the head
		currentNode = list->head;
		//sets control bit to 2 to indicate PSP used
		__set_CONTROL(1<<1);
		//sets PSP register with PSP of first thread passed as parameter
		__set_PSP((uint32_t)list->head->threadStackP);
		

		osYield();
	}
}


