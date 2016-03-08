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
	printf("G005_test: START\r\n");
	printf("G005_test: total 5 tests\r\n");
	printf("proc1 started\r\n");
	env = (msgbuf *)request_memory_block();
	env2 = (msgbuf *)request_memory_block();
	
	data = "All";
	env->mtype = DEFAULT;
	strncpy(env->mtext, data, strlen(data));
	
	send_message(2, env);
	if(gp_pcbs[1]->first_msg != NULL){
		printf("G005_test: test 1 OK\r\n");
	}
	else{
		printf("G005_test: test 1 FAIL\r\n");
	}
	//delayed_send(CRT_PROC_ID, env, 150);
	
	set_process_priority(2, 1);
	
	data = "Bee";
	env2->mtype = CRT_DISPLAY;
	strncpy(env2->mtext, data, strlen(data));
	
	//delayed_send(CRT_PROC_ID, env2, 100);

	
	
	//send_message(2, env);
	
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
	char data[5];
	char data2[5];
	msgbuf* envo;
	msgbuf* envo2;
	msgbuf* env;
	char* test4all;
	char* test5all;
	char* test2all = "All";
	test4all = "UNBLK";
	test5all = "DEL";
	
	printf("proc2 started\r\n");
	envo = (msgbuf *)request_memory_block();
	envo = receive_message(&sender_id);
	strncpy(data, envo->mtext, 5);
	
	if(strcmp(data, test2all) == 0){
		printf("G005_test: test 2 OK\r\n");
	}
	else{
		printf("G005_test: test 2 FAIL\r\n");
	}

	set_process_priority(3, 2);
	envo2 = (msgbuf *)request_memory_block();
	envo2 = receive_message(&sender_id);
	strncpy(data, envo2->mtext, 5);
	
	if(strcmp(data, test4all) == 0){
		printf("G005_test: test 4 OK\r\n");
	}
	else{
		printf("G005_test: test 4 FAIL\r\n");
	}
	
	set_process_priority(3, 0);
	
	envo2 = receive_message(&sender_id);
	strncpy(data, envo2->mtext, 5);
	
	
	if(strcmp(data, test5all) == 0){
		printf("G005_test: test 5 OK\r\n");
	}
	else{
		printf("G005_test: test 5 FAIL\r\n");
	}
	//env = (msgbuf *)request_memory_block();
	//env = receive_message(&sender_id2);
	//strncpy(data2, envo->mtext, 5);
	
	while(1){
		release_processor();
	}
}

void proc3(void)
{
	msgbuf* env;
	msgbuf* env2;
	char* data;
	char* data2;
	
	printf("proc3 started\r\n");
	
	if(gp_pcbs[1]->m_state == MSG_BLK){
		printf("G005_test: test 3 OK\r\n");
	}
	else{
		printf("G005_test: test 3 FAIL\r\n");
	}
	
	env = (msgbuf *)request_memory_block();
	
	data = "UNBLK";
	env->mtype = DEFAULT;
	strncpy(env->mtext, data, strlen(data));
	send_message(2, env);
	
	env2 = (msgbuf *)request_memory_block();
	
	data2 = "DEL";
	env2->mtype = DEFAULT;
	strncpy(env2->mtext, data2, strlen(data2));
	delayed_send(2, env2, 10);
	set_process_priority(3, 2);
	
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
	printf("proc5 started\r\n");
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