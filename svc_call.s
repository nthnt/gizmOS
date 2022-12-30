	AREA handle_pend,CODE,READONLY ;creates area that holds important code in a single contiguous block of space
	GLOBAL PendSV_Handler ;declare PendSV interrupt function globally
	PRESERVE8 ;tells linker that stack will lie on 8 boundary

PendSV_Handler
	MOV LR,#0xFFFFFFFD ;return from interrupt via laoding constant into handler, switches from MSP to PSP and go back to thread mode
	BX LR ;branch to LR and maintain certain processor state information
	END ;end file
