/**
 * @file:   k_process.c  
 * @brief:  process management C file
 * @author: Yiqing Huang
 * @author: Thomas Reidemeister
 * @date:   2014/02/28
 * NOTE: The example code shows one way of implementing context switching.
 *       The code only has minimal sanity check. There is no stack overflow check.
 *       The implementation assumes only two simple user processes and NO HARDWARE INTERRUPTS. 
 *       The purpose is to show how context switch could be done under stated assumptions. 
 *       These assumptions are not true in the required RTX Project!!!
 *       If you decide to use this piece of code, you need to understand the assumptions and
 *       the limitations. 
 */

#include <LPC17xx.h>
#include <system_LPC17xx.h>
#include "uart_polling.h"
#include "k_process.h"
#include "p_queue.h"
#include "rtx.h"
#include "timer.h"
#include "usr_proc.h"
#include "sys_proc.h"
#include "i_proc.h"

#ifdef DEBUG_0
#include "printf.h"
#endif /* DEBUG_0 */

/* ----- Global Variables ----- */
PCB **gp_pcbs;                  /* array of pcbs */
PCB *gp_current_process = NULL; /* always point to the current RUN process */

U32 g_switch_flag = 0;          /* whether to continue to run the process before the UART receive interrupt */
                                /* 1 means to switch to another process, 0 means to continue the current process */
				/* this value will be set by UART handler */

/* process initialization table */
PROC_INIT2 g_proc_table[NUM_TEST_PROCS + NUM_SYS_PROCS + NUM_I_PROCS];
extern PROC_INIT2 g_test_procs[NUM_TEST_PROCS];
extern PROC_INIT2 g_sys_procs[NUM_SYS_PROCS];
extern PROC_INIT2 g_i_procs[NUM_I_PROCS];

/**
 * @biref: initialize all processes in the system
 * NOTE: We assume there are only two user processes in the system in this example.
 */
 
 //Process queues
p_queue priority_q[NUM_PRI];

void process_init()
{
    int i;
    uint32_t *sp;
		int priority;
    
    /* fill out the initialization table */
	
    set_test_procs();
    for ( i = 0; i < NUM_TEST_PROCS; i++ ) {
        g_proc_table[i].m_pid = g_test_procs[i].m_pid;
				g_proc_table[i].m_priority = g_test_procs[i].m_priority;
        g_proc_table[i].m_stack_size = g_test_procs[i].m_stack_size;
        g_proc_table[i].mpf_start_pc = g_test_procs[i].mpf_start_pc;
    }
		
		set_sys_procs();
		for ( i = 0; i < NUM_SYS_PROCS; i++) {
				g_proc_table[i+ NUM_TEST_PROCS].m_pid = g_sys_procs[i].m_pid;
				g_proc_table[i+ NUM_TEST_PROCS].m_priority = g_sys_procs[i].m_priority;
        g_proc_table[i+ NUM_TEST_PROCS].m_stack_size = g_sys_procs[i].m_stack_size;
        g_proc_table[i+ NUM_TEST_PROCS].mpf_start_pc = g_sys_procs[i].mpf_start_pc;
		}
		
		set_i_procs();
		for ( i = 0; i < NUM_I_PROCS; i++) {
				g_proc_table[i+ NUM_TEST_PROCS + NUM_SYS_PROCS].m_pid = g_i_procs[i].m_pid;
				g_proc_table[i+ NUM_TEST_PROCS + NUM_SYS_PROCS].m_priority = g_i_procs[i].m_priority;
        g_proc_table[i+ NUM_TEST_PROCS + NUM_SYS_PROCS].m_stack_size = g_i_procs[i].m_stack_size;
        g_proc_table[i+ NUM_TEST_PROCS + NUM_SYS_PROCS].mpf_start_pc = g_i_procs[i].mpf_start_pc;
		}
    
    /* initilize exception stack frame (i.e. initial context) for each process */
    for ( i = 0; i < NUM_TEST_PROCS + NUM_SYS_PROCS + NUM_I_PROCS; i++ ) {
        int j;
        (gp_pcbs[i])->m_pid = (g_proc_table[i]).m_pid;
				(gp_pcbs[i])->m_priority = (g_proc_table[i]).m_priority;
        (gp_pcbs[i])->m_state = NEW;
				(gp_pcbs[i])->first_msg = NULL;
				(gp_pcbs[i])->last_msg = NULL;
        
        sp = alloc_stack((g_proc_table[i]).m_stack_size);
        *(--sp)  = INITIAL_xPSR;      // user process initial xPSR
        *(--sp)  = (uint32_t)((g_proc_table[i]).mpf_start_pc); // PC contains the entry point of the process
        for ( j = 0; j < 6; j++ ) { // R0-R3, R12 are cleared with 0
            *(--sp) = 0x0;
        }
        (gp_pcbs[i])->mp_sp = sp;
    }
		
		for (i = 0; i < NUM_PRI; ++i)
    {
        p_queue_init(&priority_q[i]);    
    }
		
    for ( i = 0; i < NUM_TEST_PROCS + NUM_SYS_PROCS; i++ ) {
        priority = gp_pcbs[i]->m_priority;

        p_enqueue(&priority_q[priority], gp_pcbs[i]);
    }
		
		/*
	timer_i_pcb->mp_sp = NULL;	
	timer_i_pcb->m_priority = 0; 
	timer_i_pcb->m_pid = TIME_PROC_ID;		
	timer_i_pcb->m_state = RUN;   
	timer_i_pcb->next = NULL; 
	timer_i_pcb->prev = NULL; 
	timer_i_pcb->first_msg = NULL;
	timer_i_pcb->last_msg = NULL;*/
		
	//gp_pcbs[TIME_PROC_ID-1] = timer_i_pcb;
}

/*@brief: scheduler, pick the pid of the next to run process
 *@return: PCB pointer of the next to run process
 *         NULL if error happens
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */

PCB *scheduler(void)
{
	int id;
	int i;
	PCB* nextProc;
	
	/*if (gp_current_process == NULL) {
		gp_current_process = gp_pcbs[0]; 
		return gp_pcbs[0];
	}*/
	
	
	 for ( i = 0; i < 4; i++ ) {
		 nextProc = (PCB*)p_findproc(&priority_q[i]);

		 if(nextProc != NULL && nextProc != gp_current_process){
			  gp_current_process = nextProc;
			 return gp_current_process;
		 }
	}
	return NULL;
	

/*	if ( gp_current_process == gp_pcbs[0] ) {
		return gp_pcbs[1];
	} else if ( gp_current_process == gp_pcbs[1] ) {
		return gp_pcbs[0];
	} else {
		return NULL;
	}*/
}

/*@brief: switch out old pcb (p_pcb_old), run the new pcb (gp_current_process)
 *@param: p_pcb_old, the old pcb that was in RUN
 *@return: RTX_OK upon success
 *         RTX_ERR upon failure
 *PRE:  p_pcb_old and gp_current_process are pointing to valid PCBs.
 *POST: if gp_current_process was NULL, then it gets set to pcbs[0].
 *      No other effect on other global variables.
 */
int process_switch(PCB *p_pcb_old) 
{
	PROC_STATE_E state;
	
	state = gp_current_process->m_state;

	if (state == NEW) {
		if (gp_current_process != p_pcb_old && p_pcb_old->m_state != NEW) {
			if(p_pcb_old->m_state != BLK && p_pcb_old->m_state != MSG_BLK){
							p_pcb_old->m_state = RDY;	
			}
				p_pcb_old->mp_sp = (U32 *) __get_MSP();		
		}
		gp_current_process->m_state = RUN;
		__set_MSP((U32) gp_current_process->mp_sp);
		__rte();  // pop exception stack frame from the stack for a new processes
	} 
	
	/* The following will only execute if the if block above is FALSE */

	if (gp_current_process != p_pcb_old) {
		if (state == RDY){ 		
			if(p_pcb_old->m_state != BLK && p_pcb_old->m_state != MSG_BLK) {
				p_pcb_old->m_state = RDY; 
			}

			p_pcb_old->mp_sp = (U32 *) __get_MSP(); // save the old process's sp
			gp_current_process->m_state = RUN;
			__set_MSP((U32) gp_current_process->mp_sp); //switch to the new proc's stack   			
		} else {
			gp_current_process = p_pcb_old; // revert back to the old proc on error
			return RTX_ERR;
		} 
	}
	return RTX_OK;
}
/**
 * @brief release_processor(). 
 * @return RTX_ERR on error and zero on success
 * POST: gp_current_process gets updated to next to run process
 */
int k_release_processor(void)
{
	
	PCB *p_pcb_old = NULL;
	
	p_pcb_old = gp_current_process;
	gp_current_process = scheduler();
	
	if ( gp_current_process == NULL  ) {
		gp_current_process = p_pcb_old; // revert back to the old process
		return RTX_ERR;
	}
        if ( p_pcb_old == NULL ) {
		p_pcb_old = gp_current_process;
	}
	
	process_switch(p_pcb_old);
	
	return RTX_OK;
}



int set_process_priority(int process_id, int priority)
{
    PCB* proc;
	  PCB* nextProc;
    int old_priority;
		int i;
		int id;
		id = process_id - 1;
	
		if (process_id < 1 || process_id > (NUM_TEST_PROCS+NUM_SYS_PROCS+NUM_I_PROCS+1) || priority < 0 || priority > 3){
			//invalid proc_id or invalid priority
			return 1;
		}
	
    proc = gp_pcbs[id];
    old_priority = proc->m_priority;

    p_queue_remove(&priority_q[old_priority], id);

    proc->m_priority = priority;
    p_enqueue(&priority_q[priority], proc);
	
		
		for ( i = 0; i < 4; i++ ) {
		 nextProc = (PCB*)p_findAllproc(&priority_q[i]);
		 if(nextProc != NULL ){
					if(nextProc != gp_current_process){
							release_processor();
							break;
					}
					else{
							break;
					}
			}
		}

		return 0;
}



int get_process_priority(int process_id)
{
		int ret;
		int id;
		PCB* proc;
		id = process_id - 1;
    proc = gp_pcbs[id];
		ret = proc->m_priority;
    return ret;
}

int notify_mem_released(void) {
	int i;
	PCB * proc;
	
	for ( i = 0; i < 4; i++ ) {
		 proc = (PCB*)p_findblockedproc(&priority_q[i]);
		 if(proc != NULL){
				proc->m_state = RDY;
			 if(proc->m_priority <= gp_current_process->m_priority){
						return 1;
			 }
		 }
	 }
	 
	 return 0;
}

void unblock_proc(int id) {
		gp_pcbs[id -1]->m_state = RDY;
}

void block_proc(int id){
		gp_pcbs[id -1]->m_state = BLK;
}
