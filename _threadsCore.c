//This file contains relevant pin and other settings 
#include <LPC17xx.h>

//This file is for printf and other IO functions
#include "stdio.h"

//thread libraries
#include "_threadsCore.h"
#include "osDefs.h"

//for malloc
#include <stdlib.h>

//for user-side functions
#include <stdarg.h>

//keep track of the thread number
//extern int tasknum;
//dynamic linked list to store threads
extern threadLinkedList* list;

//create currentNode pointer to reference the head node later
extern threadNode* currentNode;

uint32_t* getMSPInitialLocation(void) {
	uint32_t* p = 0x0;
	return (uint32_t*)*p; //dereference p to get MSP location (as integer) at 0x0 then cast to pointer 
}

//create a new thread
void newThread(void (*threadFunc) (void* args)) {
	//create new thread
	threadNode* thread = malloc(sizeof(struct threadNode));
	
	//assign characteristics of thread
	thread->threadFunc = threadFunc;
	thread->threadStackP = getNewThreadStack(MSR_SIZE + (list->size)*PSP_SIZE);
	thread->next = NULL;
	thread->status = ACTIVE;
	thread->runTimer = OS_RUNTIME;
	// on new thread creation, the thread does not have any mutex to wait on
	thread->mutexID = -1; 
	
	//add the particular thread to the linked list
	if (list->head == NULL) {
		// when the first thread is added, its next is pointing to itself
		list->head = thread;
		thread->next = list->head;
		list->tail = thread;
	} else if (list->size < MAXNUMTHREADS) {
		// new threads will be added at the head
		thread->next = list->head;
		list->head = thread;
		
		// tail's next is the head
		list->tail->next = list->head;
	}
	
	//increment the list size
	list->size++;
	
	currentNode = list->head;
	
	//set thread mode
	*(--(currentNode->threadStackP)) = 1<<24;
	//store PC
	*(--currentNode->threadStackP) = (uint32_t) threadFunc;
	//store registers
	*(--currentNode->threadStackP) = 0x8; //LR
	*(--currentNode->threadStackP) = 0x7; //R12
	*(--currentNode->threadStackP) = 0xD; //R3
	*(--currentNode->threadStackP) = 0xC; //R2
	*(--currentNode->threadStackP) = 0xB; //R1
	*(--currentNode->threadStackP) = 0xA; // R0
	*(--currentNode->threadStackP) = 0x6; //R11
	*(--currentNode->threadStackP) = 0x5; //R10
	*(--currentNode->threadStackP) = 0x4; //R9
	*(--currentNode->threadStackP) = 0x3; //R8
	*(--currentNode->threadStackP) = 0x2; //R7
	*(--currentNode->threadStackP) = 0x1; //R6
	*(--currentNode->threadStackP) = 0xF; //R5
	*(--currentNode->threadStackP) = 0xE; //R4

}

uint32_t* getNewThreadStack(uint32_t offset){
	uint32_t msp_int = (uint32_t) getMSPInitialLocation();
	uint32_t psp_int = msp_int - offset;
	
	if(psp_int%8)
		psp_int+=(psp_int - psp_int*8);
	
	//error checking and returns null if the offset is greater than size
	//of max stack (8192 bytes)
	if(msp_int - psp_int > 8192) {
		return NULL;
	}
	
	return (uint32_t*) psp_int;
}

//switches between threads
int task_switch(void){
	__set_PSP((uint32_t)currentNode->threadStackP);
	
	return 1;
}
