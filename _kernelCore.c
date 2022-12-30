#include "_kernelCore.h"

//for malloc
#include <stdlib.h>
#include <stdio.h>

// task counter to determine current and next thread
int tasknum = 0;

// dynamic linked list to store threads
threadLinkedList* list;

// data node of the current thread
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
	//we always start at the head thread of the linkedlist by design
	currentNode = list->head;
	
	//tasknum at this point is the current thread's number
	//iterate to the current thread via task number
	for (int i = 0; i < tasknum; i++) {
			currentNode = currentNode->next;
	}
	
	//set the current status to waiting and make space for its registers relative to current PSP
	if (tasknum >= 0) {
		currentNode->status = WAITING;
	    currentNode->threadStackP = (uint32_t*)(__get_PSP() - 16*4); //we are about to push 16 uint32_t's
	}
	
	
	// tasknum is incremented to become the next thread
	if (tasknum + 1 == list->size) { //reset taskNum if last thread is reached
		tasknum = 0;
		currentNode = list->head;
	} else {
		tasknum++;
		currentNode = currentNode->next;
	}
	
	//set the next node's status to be active and running
	currentNode->status = ACTIVE;
	
	//pend a context switch using the Interrupt Control and State Register
	//this will trigger PendSV in svc_calls.s
	_ICSR |= 1<<28;
	__asm("isb"); //flush the pipeline
}

int osKernelStart(void) {
	//check if any threads created and if so, set the first thread to run to avoid bootstrapping
	if(list->size > 0) {
		//set to -1, this skips iterating to the next thread in osYield 
		//and storing the current PSP, and just starts up the head thread
		tasknum = -1;
		//sets control bit to 2 to indicate PSP used
		__set_CONTROL(1<<1);
		//sets PSP register with PSP of first thread passed as parameter
		__set_PSP((uint32_t)list->head->threadStackP);
		osYield();
	}
	return 0;
}

//switches between threads
int task_switch(void){
	currentNode = list->head;
	
	//go to thread with corresponding task number
	//when inside this scope, tasknum is next thead
	for (int i = 0; i < tasknum; i++) {
			currentNode = currentNode->next;
	}
	
	//set the new PSP
	__set_PSP((uint32_t)currentNode->threadStackP);
	
	return 1;
}
