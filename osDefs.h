#ifndef OSDEFS_H
#define OSDEFS_H

#include <stdint.h>

#define MAXNUMTHREADS 5
#define MSR_SIZE 0x400
#define PSP_SIZE 0x200

// defines the various states of the threads
#define CREATED 0
#define WAITING 1
#define ACTIVE 2
#define DESTROYED 3

//thread struct to hold thread details such as stack pointer, function pointer, and status
typedef struct threadNode {
	uint32_t* threadStackP;
	void (*threadFunc)(void* args);
	int status;
	
	//defining thread in recursive manner
	struct threadNode* next;
} threadNode;

//define linked list for thread nodes
typedef struct threadLinkedList {
	threadNode* head;
	int size;
} threadLinkedList;

#endif
