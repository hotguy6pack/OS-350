/**
 * @file:   usr_proc.c
 * @brief:  Two user processes: proc1 and proc2
 * @author: Yiqing Huang
 * @date:   2014/02/28
 * NOTE: Each process is in an infinite loop. Processes never terminate.
 */

#include "rtx.h"
#include "uart_polling.h"
#include "usr_proc.h"
#include "k_process.h"
#include "k_message.h"
#include "sys_proc.h"
#ifdef DEBUG_0
#include "printf.h"

#endif /* DEBUG_0 */

/* initialization table item */
PROC_INIT g_test_procs[NUM_TEST_PROCS];
int numTestTotal;
int numTestPassed;

void set_test_procs() {
	int i;
	numTestTotal = 0;
	numTestPassed = 0;
	
	
	for( i = 0; i < NUM_TEST_PROCS; i++ ) {
		g_test_procs[i].m_pid=(U32)(i+1);
		g_test_procs[i].m_priority=LOWEST;
		g_test_procs[i].m_stack_size=0x100;
	}
	
	g_test_procs[0].m_priority=2;
	//g_test_procs[1].m_priority=2;
	//g_test_procs[2].m_priority=2;
	
	g_test_procs[0].mpf_start_pc = &proc1;
	g_test_procs[1].mpf_start_pc = &proc2;
	g_test_procs[2].mpf_start_pc = &proc3;
	g_test_procs[3].mpf_start_pc = &proc4;
	g_test_procs[4].mpf_start_pc = &proc5;
	g_test_procs[5].mpf_start_pc = &proc6;
}


void proc0(void) {
	printf("proc0 started\r\n");
	while(1) {
		release_processor();
	}
}

/**
 * @brief: a process that prints 5x6 uppercase letters
 *         and then yields the cpu.
 */
void proc1(void)
{
	int i = 0;
	printf("proc1 started\r\n");
	while(1){
		if(i % 10000 == 0){
			i = 0;
			printf("proc1 started\r\n");
			release_processor();
		}
		i++;
		
	}
}

/**
 * @brief: a process that prints 5x6 numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	int i = 0;
	while(1){
		if(i % 10000 == 0){
			i = 0;
			printf("proc2 started\r\n");
			release_processor();
		}
		i++;
		
	}
}

void proc3(void)
{
	int i = 0;
	while(1){
		if(i % 10000 == 0){
			i = 0;
			printf("proc3 started\r\n");
			release_processor();
		}
		i++;
		
	}
}

void proc4(void)
{
	int i = 0;
	while(1){
		if(i % 10000 == 0){
			i = 0;
			printf("proc4 started\r\n");
			release_processor();
		}
		i++;
		
	}
}

void proc5(void)
{
	int i = 0;
	while(1){
		if(i % 10000 == 0){
			i = 0;
			printf("proc5 started\r\n");
			release_processor();
		}
		i++;
		
	}
}

void proc6(void)
{
	int i = 0;
	while(1){
		if(i % 10000 == 0){
			i = 0;
			printf("proc6 started\r\n");
			release_processor();
		}
		i++;
		
	}
}