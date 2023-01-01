#ifndef OSDEFS_H
#define OSDEFS_H

#include <stdint.h>
#include <stdbool.h>
#include <limits.h>

#define MAXNUMTHREADS 5
#define MAXNUMMUTEX 5
#define MSR_SIZE 0x400
#define PSP_SIZE 0x200

// defines the various states of the threads
#define ACTIVE 0 // ready to run but not the thread's turn
#define RUNNING 1 // is currently running
#define WAITING 2 // not ready to run, waiting on mutex

// define OS runtime in ms
#define OS_RUNTIME 3
#define OS_WAITFOREVER INT_MAX //wait forever on a mutex

// define mutex constants
#define ACQUIRE_SUCCESS 0
#define ACQUIRE_FAILURE 1

//thread struct to hold thread details such as stack pointer, function pointer, and status
typedef struct threadNode{
	uint32_t* threadStackP;
	void (*threadFunc)(void* args);
	uint32_t status;
	// amount of time before the thread is forced to pre-emptively yield
	uint32_t runTimer;
	
	// MUTEXES, variables used to manage mutexes
	uint32_t mutexTimer; // amount of time willing to wait on mutex
	uint32_t mutexID; // mutex the thread is waiting on

	//defining thread in recursive manner
	struct threadNode* next;
} threadNode;

//define linked list for thread nodes
typedef struct threadLinkedList {
	threadNode* head;
	threadNode* tail;
	int size;
} threadLinkedList;

// mutex queue data structure, stores pointers to the threadnodes in the thread linked list
typedef struct mutexQueueNode {
	struct mutexQueueNode* next;
	threadNode* tNode;
}mutexQueueNode;

//thread waiting queue for threads waiiting on a mutex
typedef struct mutexQueue {
	mutexQueueNode* head;
} mutexQueue;

//define mutex linked list
typedef struct mutex{
	bool available;
	mutexQueue* threadQ; //mutex.list->head
} mutex;

#endif
