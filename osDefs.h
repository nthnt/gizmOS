#ifndef OSDEFS_H
#define OSDEFS_H

#include <stdint.h>

#define MAXNUMTHREADS 5
#define MSR_SIZE 0x400
#define PSP_SIZE 0x200

// defines the various states of the threads
#define ACTIVE 0
#define SLEEPING 1
#define RUNNING 2

// define OS runtime in ms
#define OS_RUNTIME 5

//thread struct to hold thread details such as stack pointer, function pointer, and status
typedef struct threadNode{
	uint32_t* threadStackP;
	void (*threadFunc)(void* args);
	uint32_t status;
	uint32_t runTimer;
	uint32_t sleepTimer;
	
	//defining thread in recursive manner
	struct threadNode* next;
} threadNode;

//define linked list for thread nodes
typedef struct threadLinkedList {
	threadNode* head;
	threadNode* tail;
	int size;
} threadLinkedList;

#endif
