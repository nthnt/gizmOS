#ifndef OSDEFS_H
#define OSDEFS_H

#include <stdint.h>

#define MAXNUMTHREADS 5
#define INF_DEADLINE 0xFFFFFFF //large deadline constant (primarily used for idle task)
#define MSR_SIZE 0x400
#define PSP_SIZE 0x200

// defines the various states of the threads
#define ACTIVE 0 //ready to run but not the thread's turn
#define SLEEPING 1
#define RUNNING 2
//#define WAITING 3 //not ready to run

// define OS runtime in ms
#define OS_RUNTIME 5

//thread struct to hold thread details such as stack pointer, function pointer, and status
typedef struct threadNode{
	uint32_t* threadStackP;
	void (*threadFunc)(void* args);
	uint32_t status;
	// amount of time before the thread is forced to pre-emptively yield
	uint32_t runTimer;
	// amount of time the thread is specified to sleep for (also includes the period timer)
	uint32_t sleepTimer;
	// period of the thread (specified by user)
  	uint32_t period;
	// deadline of the thread (specified by user)
	uint32_t deadline;
	// amount of time the thread has to complete its task
	uint32_t deadlineTimer;
	
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
