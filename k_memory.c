/**
 * @file:   k_memory.c
 * @brief:  kernel memory managment routines
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */

#include "k_memory.h"
#include "k_process.h"
#include "sys_proc.h"
#include "k_rtx.h"
#include "timer.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* ! DEBUG_0 */

/* ----- Global Variables ----- */
U32 *gp_stack; /* The last allocated stack low address. 8 bytes aligned */
               /* The first stack starts at the RAM high address */
	       /* stack grows down. Fully decremental stack */

/**
 * @brief: Initialize RAM as follows:

0x10008000+---------------------------+ High Address
          |    Proc 1 STACK           |
          |---------------------------|
          |    Proc 2 STACK           |
          |---------------------------|<--- gp_stack
          |                           |
          |        HEAP               |
          |                           |
          |---------------------------|
          |        PCB 2              |
          |---------------------------|
          |        PCB 1              |
          |---------------------------|
          |        PCB pointers       |
          |---------------------------|<--- gp_pcbs
          |        Padding            |
          |---------------------------|  
          |Image$$RW_IRAM1$$ZI$$Limit |
          |...........................|          
          |       RTX  Image          |
          |                           |
0x10000000+---------------------------+ Low Address

*/

mem_block* free_mem;
mem_block* starve_mem;

int starve =0;

U32 start_addr;
U32 stack_size;


//int starved_clock;

void memory_init(void)
{
	U8 *p_end = (U8 *)&Image$$RW_IRAM1$$ZI$$Limit;
	int i;
	mem_block* current;
	mem_block* m;

	/* 4 bytes padding */
	p_end += 4;


	
	start_addr = RAM_TOP - ((NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + NUM_I_PROCS) * USR_SZ_STACK);
	free_mem = (mem_block*) (start_addr - MEM_BLK_SZ);
	
	current = free_mem;
	
	for (i = 1; i < NUM_MEM_BLKS; ++i) {
		m = (mem_block*) ((U32)free_mem - (i * MEM_BLK_SZ));
		m->next = NULL;
		current->next = m;
		current = m;
	}
	
	p_end = p_end + (COMMAND_REG_SIZE * COMMAND_REG_NUM);
	
	/* allocate memory for pcb pointers   */
	gp_pcbs = (PCB **)p_end;
	p_end += (NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + NUM_I_PROCS) * sizeof(PCB *);
  
	for ( i = 0; i < NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + NUM_I_PROCS; i++ ) {
		gp_pcbs[i] = (PCB *)p_end;
		p_end += sizeof(PCB); 
	}
	
	
	command_head = (command_registry*) p_end;
	
#ifdef DEBUG_0  
	printf("gp_pcbs[0] = 0x%x \n", gp_pcbs[0]);
	printf("gp_pcbs[1] = 0x%x \n", gp_pcbs[1]);
#endif
	
	/* prepare for alloc_stack() to allocate memory for stacks */
	
	gp_stack = (U32 *)RAM_END_ADDR;
	if ((U32)gp_stack & 0x04) { /* 8 bytes alignment */
		--gp_stack; 
	}
  
	/* allocate memory for heap, not implemented yet*/
  
	//m = (struct mem_block *) 0x10007000;//(struct mem_block *)p_end + 0x8;
	//m->next = (void *)0xABCDABCD;
}

/**
 * @brief: allocate stack for a process, align to 8 bytes boundary
 * @param: size, stack size in bytes
 * @return: The top of the stack (i.e. high address)
 * POST:  gp_stack is updated.
 */

U32 *alloc_stack(U32 size_b) 
{
	U32 *sp;
	sp = gp_stack; /* gp_stack is always 8 bytes aligned */
	
	/* update gp_stack */
	gp_stack = (U32 *)((U8 *)sp - size_b);
	
	/* 8 bytes alignement adjustment to exception stack frame */
	if ((U32)gp_stack & 0x04) {
		--gp_stack; 
	}
	return sp;
}

void *k_request_memory_block(void) {
 unsigned int end_addr = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;	
	mem_block* req_block = NULL;
	//mem_block* temp_block = used_mem;
	
	while(free_mem == NULL)
	{
		//printf("BLOCK HERE\r\n");
		// assign PCB to the linked list of blocked processes
		
		//block_current_process();
		gp_current_process->m_state=BLK;
		
		// set the process state flag to PROC_STATE_E.BLK
		k_release_processor();
	}
	
	req_block = free_mem;

	free_mem = (mem_block*)free_mem->next;
	
	//printf("assigned mem_block: 0x%08x\r\n", (int)req_block);
	return req_block;
}

void *k_request_memory_block_i(void) {
	int i;
	unsigned int end_addr = (unsigned int) &Image$$RW_IRAM1$$ZI$$Limit;	
	mem_block* req_block = NULL;
	//mem_block* temp_block = used_mem;
	
	if(free_mem == NULL){
		return NULL;		
	}
	
	req_block = free_mem;

	free_mem = (mem_block*)free_mem->next;
	
	for(i = (int)req_block; i < (int)req_block + MEM_BLK_SZ; i += 4){
		*((int*)i) = '\0';
	}
	
	//printf("assigned mem_block: 0x%08x\r\n", (int)req_block);
	return req_block;
}

int is_valid_mem_blk( void* p_mem_blk ){
	int i;
	int ret = 0;
	int first_valid;
	
	start_addr = RAM_TOP - ((NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + NUM_I_PROCS) * USR_SZ_STACK);
	first_valid = start_addr - MEM_BLK_SZ;
	for (i = 0; i < NUM_MEM_BLKS; ++i){
		if ((unsigned int) p_mem_blk == first_valid - i * MEM_BLK_SZ)
			ret = 1;
	}
	return ret;
}

int k_release_memory_block(void *p_mem_blk) {
	int ret;
	int rel;
	mem_block* temp;
	
	if (!is_valid_mem_blk(p_mem_blk)){
		ret = RTX_ERR;
	}
	else
	{
			// if we have a blocked proc, assign this mem address to it
	// else put the mem_blk into heap
	
		if (starved_clock == 0) {
			rel = notify_mem_released();
			
		}
		temp = (mem_block*) p_mem_blk;
		temp->next = (mem_block *)free_mem;
		free_mem = temp;
		
		//printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
		ret = 0;
		

		if(rel == 1){
					k_release_processor();
		}
	}
	

  return ret;
}

int k_release_memory_block_i(void *p_mem_blk) {
	int ret;
	int rel =0;
	mem_block* temp;
	
	
	if (!is_valid_mem_blk(p_mem_blk)){
		ret = RTX_ERR;
	}
	else
	{
			// if we have a blocked proc, assign this mem address to it
	// else put the mem_blk into heap
		

		if (starved_clock == 0) {
			rel = notify_mem_released();
		}
		temp = (mem_block*) p_mem_blk;
		temp->next = (mem_block *)free_mem;
		free_mem = temp;
		
		//printf("k_release_memory_block: releasing block @ 0x%x\n", p_mem_blk);
	}
	

  return rel;
}
