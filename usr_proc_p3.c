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
	msgbuf* env;
	msgbuf* env2;
	char* data;
	int actual;
	int expected = 2;
	

	env = (msgbuf *)request_memory_block();
	
	data = "%C 1 2";
	env->mtype = DEFAULT;
	strncpy(env->mtext, data, strlen(data));

	send_message(SET_PRIORITY_PROC_ID, env);
	
	actual = get_process_priority(1);
	
	if (actual == expected){
			printf("G006_test: test 1 OK\r\n");
	}
	
	env2 = (msgbuf *)request_memory_block();
	data = "%C 2 1";
	env2->mtype = DEFAULT;
	strncpy(env2->mtext, data, strlen(data));
	
	send_message(SET_PRIORITY_PROC_ID, env2);

	expected = 3;
	actual = get_process_priority(2);
	
	if (actual == expected){
			printf("G006_test: test 3 OK\r\n");
	}
	while(1){
		release_processor();
	}
}

/**
 * @brief: a process that prints 5x6 numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	int sender_id;
	int sender_id2;
	char* data;
	char data2[5];
	msgbuf* envo;
	msgbuf* envo2;
	msgbuf* env;
	int actual;
	
	int expected = 1;
	actual = get_process_priority(2);
	
	if (actual == expected){
			printf("G006_test: test 2 OK\r\n");
	}
	
	env = (msgbuf *)request_memory_block();
	
	data = "%C 2 3";
	env->mtype = DEFAULT;
	strncpy(env->mtext, data, strlen(data));

	send_message(SET_PRIORITY_PROC_ID, env);
	
	while(1){
		release_processor();
	}
}

void proc3(void)
{
	printf("proc3 started\r\n");
	while(1){
		release_processor();
	}
}

void proc4(void)
{
	printf("proc4 started\r\n");
	while(1){
		release_processor();
	}
}

void proc5(void)
{
	int sender_id;
	int sender_id2;
	while(1){
		release_processor();
	}
}

void proc6(void)
{
	printf("proc6 started\r\n");
	while(1){
		release_processor();
	}
}