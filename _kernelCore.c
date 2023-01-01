#include "_kernelCore.h"

//for malloc
#include <stdlib.h>
#include <stdio.h>

bool startup = false;
uint32_t mutexIDCount = 0;
threadLinkedList* list;
threadNode* currentNode;
mutex mutexes[MAXNUMMUTEX];


void idle (void* args)
{
	while(1)
	{
		printf("idle thread %x\n",__get_PSP());
	}
}

uint32_t osMutexCreate(void) {
	//initialize mutex to be tru
	mutexes[mutexIDCount].available = true;
	//save the value fo the initialized mutex
	uint32_t initializedMutexID = mutexIDCount;
	//dynamically allocate queue for waiting threads
	mutexes[mutexIDCount].threadQ = malloc(sizeof(mutexQueue));
	mutexes[mutexIDCount].threadQ->head = NULL;

	//increment id count for future mutex initialization calls
	mutexIDCount++;

	//return the value of the intiialized mutex
	return initializedMutexID;
}

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
	//SHPR3 |= 0xFF << 16;
	SHPR3 |= 0xFE << 16;
	SHPR3 |= 0xFFU << 24; //Set the priority of SysTick to be the weakest
	SHPR2 |= 0xFDU << 24; //Set the priority of SVC the be the strongest
	
	//initialize idle thread in kernelInit, this is not modifiable by the user
	newThread(idle);
	
	//initialize max number of mutexes
	for (int i = 0; i < MAXNUMMUTEX; i++) {
		osMutexCreate();
	}
}

void osYield(void) {
	__ASM("SVC #0");
}

void SVC_Handler_Main(uint32_t *svc_args) {
	//accessing stack frame pushed onto the correct stack when SVC interrupt happened
	
	char call = ((char*)svc_args[6])[-2];
	//Now your system call stuff looks at the value of “call” and does what
	//ever it needs to based on that information
	
	currentNode->runTimer = OS_RUNTIME;

	//set the current status to active and make space for its registers relative to current PSP
	if (!startup) {
		if (currentNode->status != WAITING){
			currentNode->status = ACTIVE;
		}
		currentNode->threadStackP = (uint32_t*)(__get_PSP() - 8*4); //we are about to push 16 uint32_t's
		
		// scheduler returns the next thread to run
		threadNode* schedNode = scheduler();
		if (schedNode == currentNode) {
			currentNode->status = RUNNING;
			return; // if thread is the same, do nothing
		}
		currentNode = schedNode;
	}
	else {
		startup = false;
	}
	
	//set the current node's status to be active and running
	currentNode->status = RUNNING;
	
	//set state to pending via bit 28 at defined memory address
	ICSR |= 1<<28;
	//flush pipeline
	__asm("isb");
}

uint32_t osAcquireMutex (uint32_t mutexID, uint32_t timeout) {
	mutexQueueNode* dummyQNode = NULL;
	// if mutex is available, then thread acquires it
	if (mutexes[mutexID].available) {
		mutexes[mutexID].available = false;
		return ACQUIRE_SUCCESS;
	}
	
	// otherwise, thread will have to wait
	// what we will be returning, whether we aquired the mutex or not
	uint16_t acquired = ACQUIRE_FAILURE;
	
	// init mutex queue node should we need it
	mutexQueueNode* node = malloc(sizeof(mutexQueueNode));
	node->next = NULL;
	node->tNode = currentNode;	
	
	// traverse to back of mutex queue and add mutex queue node
	if (mutexes[mutexID].threadQ->head == NULL) {
		mutexes[mutexID].threadQ->head = node;
	}
	else {
		dummyQNode = mutexes[mutexID].threadQ->head;
		while(dummyQNode->next != NULL) {
			dummyQNode = dummyQNode->next;
		}
		dummyQNode->next = node;
	}
	
	currentNode->mutexTimer = timeout; // set the timeout
	currentNode->status = WAITING; // set waiting status
	currentNode->mutexID = mutexID; // set what mutex the node wants
	
	// now waiting, switch away to a running thread
	osYield();
	
	// after done waiting, set thread to waiting on no mutex and
	currentNode->mutexID = -1; 
	// check whether we met the timeout or not
	if (currentNode->mutexTimer > 0) acquired = ACQUIRE_SUCCESS;
	
	// return aquired successvalue
	return acquired;
}

uint32_t osReleaseMutex (uint32_t mutexID) {
	if (mutexes[mutexID].available) {
		printf("cannot release a mutex that is available\n");
		return 1;
	}
	
	// sets the mutex to available, Systick will handle which thread
	// gets the mutex
	mutexes[mutexID].available = true;
	
	return 0; //successvalue
}

void SysTick_Handler(void) {
	//decrement the thread's run timer
	if (currentNode->status == RUNNING) {
		currentNode->runTimer--;
	}
	
	//decrement all the sleep timers and mutex timers 
	threadNode* dummyNode = currentNode;
	for (int x = 0; x < list->size; x++) {
		dummyNode = dummyNode->next;
		
		// if node is waiting for a mutex, decrement its wait timer
		if (dummyNode->status == WAITING) {
			if (dummyNode->mutexTimer != OS_WAITFOREVER) {
				dummyNode->mutexTimer--;
			}
				
			if(dummyNode->mutexTimer <= 0) {
				// if mutex timer is up, then the node has failed to
				// acquire the mutex and has timed out. It will be set to ACTIVE
				// and called when scheduled 
				dummyNode->status = ACTIVE;
			
				// removes the thread from the mutex waiting queue
				mutexQueueNode* dummyQNode = mutexes[dummyNode->mutexID].threadQ->head;
				if (dummyQNode->next == NULL) {
					mutexes[dummyNode->mutexID].threadQ->head = NULL;
				}
				else { // if thread not head of Q, then traverse and pop
					mutexQueueNode* prevQNode = dummyQNode;
					mutexQueueNode* nextQNode = NULL;
					while (dummyQNode->tNode != dummyNode) {
							prevQNode = dummyQNode;
							dummyQNode = dummyQNode->next;
					}
					
					nextQNode = dummyQNode->next;
					prevQNode->next = nextQNode;
					dummyQNode->next = NULL;
				}
				
				free(dummyQNode); // free up memory of the node in the thread Q  
			}
		}
		
		// check if any of the mutexes are free and nodes are waiting
		// on them
		mutexQueueNode* currentQNode = NULL;
		for(int x = 0; x < mutexIDCount; x++) {
			// if a mutex is available and a thread is waiting for it
			if (mutexes[x].available && mutexes[x].threadQ->head != NULL) {
				// set avail back to false, set thread to active, pop head of threadQ
				mutexes[x].available = false;
				currentQNode = mutexes[x].threadQ->head;
				
				currentQNode->tNode->status = ACTIVE;
				
				mutexes[x].threadQ->head = currentQNode->next;
				free(currentQNode);
			}
		}
	}

	//if the thread is out of time
	if (currentNode->runTimer <= 0) {
		//reset the run timer
		currentNode->runTimer = OS_RUNTIME;
		//set status of thread back to active and ready to be scheduled
		//printf("\n\nThread is WAITING and being set to ACTIVE: %d\n\n", currentNode->status == WAITING);
		currentNode->status = ACTIVE;
		
		currentNode->threadStackP = (uint32_t*)(__get_PSP() - 8*4); 
		
		// scheduler returns the thread to switch to
		threadNode* schedNode = scheduler();
		if (schedNode == currentNode) {
			currentNode->status = RUNNING;
			return;
		}
		currentNode = schedNode;
		// set the current thread to the thread the scheduler returns
		//currentNode = dummyNode;
		//set the current node's status to be active and running
		//printf("\n\nThread is WAITING and being set to RUNNING: %d\n\n", currentNode->status == WAITING);
		currentNode->status = RUNNING;
		//set state to pending via bit 28 at defined memory address
		ICSR |= 1<<28;
		//flush pipeline
		__asm("isb");
	}
}

threadNode* scheduler (void) {
	//printf("The current node sir BEFORE is: %p\n", currentNode);
	threadNode* dummyNode = currentNode;
	
	// if the node the scheduler moved to is waiting, we skip it
	dummyNode = dummyNode->next;
	while (dummyNode->status == WAITING){
		dummyNode = dummyNode->next;
	}
	return dummyNode;
	//printf("The current node sir AFTER is: %p\n", currentNode);

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


