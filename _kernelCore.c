#include "_kernelCore.h"

//for malloc
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

bool startup = false;

threadLinkedList* list;
threadNode* currentNode;

void idle (void* args)
{
	while(1)
	{
		printf("idle thread %x\n",__get_PSP());
	}
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
	newThread(idle, NULL, NULL);
}

void osYield(void) {
	__ASM("SVC #0");
}

void osThreadSleep(uint32_t sleeptime) {
	//sets thte current node status to sleeping
	currentNode->status = SLEEPING;
	//sets the node's sleep timer
	currentNode->sleepTimer = sleeptime;
	//switch to the next thread
	osYield();
}

void SVC_Handler_Main(uint32_t *svc_args) {
	threadNode* dummyNode = currentNode;
	//accessing stack frame pushed onto the correct stack when SVC interrupt happened
	
	char call = ((char*)svc_args[6])[-2];
	//Now your system call stuff looks at the value of “call” and does what
	//ever it needs to based on that information
	
	currentNode->runTimer = OS_RUNTIME;

	//set the current status to active and make space for its registers relative to current PSP
	if (!startup) {
		if (currentNode->status != SLEEPING){
			currentNode->status = ACTIVE;
		}
		
		// scheduler returns the next thread to run
		dummyNode = scheduler();
		if (currentNode == dummyNode) { // should scheduler return the same thread, nothing happens
			return;
		}
		
		currentNode->threadStackP = (uint32_t*)(__get_PSP() - 8*4); //we are about to push 16 uint32_t's
		currentNode = dummyNode;
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

void SysTick_Handler(void) {
	//decrement the thread's run timer
	currentNode->runTimer--;
	
	//decrement all the sleep timers and period timers if not sleeping
	threadNode* dummyNode = currentNode;
	for (int x = 0; x < list->size; x++) {
		dummyNode = dummyNode->next;
		
		// if the current node is sleeping
		if (dummyNode->status == SLEEPING){
			dummyNode->sleepTimer--;
			if (dummyNode->sleepTimer == 0) { // when its done sleeping
				dummyNode->status = ACTIVE;
				dummyNode->deadlineTimer = dummyNode->deadline; //set deadline when the thread ebcomes active
			}
		} else if (dummyNode->status != SLEEPING && dummyNode != list->tail) { //thread is not sleeping and not the idle task in the tail node
			// if its not sleeping, then its either running or active, either way we need to decrement its deadline timer
			dummyNode->deadlineTimer--; //thread is active or running and decrement its run timer
			if (dummyNode->deadlineTimer == 0) { // when its deadline is met
					dummyNode->deadlineTimer = dummyNode->deadline; //reset deadline timer
					if (dummyNode->period != NULL) { //thread has a period
						dummyNode->status = SLEEPING; //make the thread sleep/wait
						dummyNode->sleepTimer = dummyNode->period; //reset the period timer
					}
			}
		}
	}

	//if the thread is out of time
	if (currentNode->runTimer <= 0) {
		//reset the run timer
		currentNode->runTimer = OS_RUNTIME;
		//set status of thread back to active and ready to be scheduled
		currentNode->status = ACTIVE;
	}
	
	// scheduler returns the thread to switch to
	dummyNode = scheduler();
	if (currentNode != dummyNode){ // if the scheduler actually changes the thread , then call a pendsv interrupt
		//we are about to push 8 uint32_t's
		currentNode->threadStackP = (uint32_t*)(__get_PSP() - 8*4); 
		// set the current thread to the thread the scheduler returns
		currentNode = dummyNode;
		//set the current node's status to be active and running
		currentNode->status = RUNNING;
		//set state to pending via bit 28 at defined memory address
		ICSR |= 1<<28;
		//flush pipeline
		__asm("isb");
	}
}

threadNode* scheduler (void) {
	threadNode* dummyNode = currentNode;
	//min is set to such to include the idle thread as well
	int min = INF_DEADLINE + 1, minIndex = 0;
	
	for (int i = 0; i < list->size; i++) { //go through all nodes
		if (dummyNode->status != SLEEPING) { //check deadlines of all non-sleeping nodes and record the minimum deadline and index
			if (dummyNode->deadlineTimer < min) {
				min = dummyNode->deadlineTimer;
				minIndex = i;
			}
		}
		
		dummyNode = dummyNode->next;
	}
	
	// return the node that has the closest deadline
	for (int a = 0; a < minIndex; a++) {
		dummyNode = dummyNode->next;
	}
	
	return dummyNode;
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


