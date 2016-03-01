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
	
  g_test_procs[0].m_priority=NULLPROC;
	
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
	int i;
	int temp;
	void* tmppt;
	uart0_put_string("G006_test: START\r\n");
	uart0_put_string("G006_test: total 6 tests\r\n");
	printf("proc1 started\r\n");
	temp = set_process_priority(3, 1);
	
	temp = set_process_priority(1, 3);
	
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
	void* tmppt;
	
	printf("proc2 started\r\n");
	
	tmppt = request_memory_block();
	printf("proc2: tmppt = 0x%08x\r\n", tmppt);
	
	if(tmppt == (void*)0x10006300){
			uart0_put_string("G006_test: test 4 OK\r\n");
			numTestPassed++;
	} else {
			uart0_put_string("G006_test: test 4 FAIL\r\n");
	}
	
	set_process_priority(3, 3);
	if(get_process_priority(3) == 3){
			uart0_put_string("G006_test: test 5 OK\r\n");
					numTestPassed++;
	} else {
			uart0_put_string("G006_test: test 5 FAIL\r\n");
	}
	

	
	
	set_process_priority(3, 0);
	
	set_process_priority(2, 3);
	while(1){
		release_processor();
	}
}

void proc3(void)
{
	int i;
	int temp;
	void* tmppt;
	printf("proc3 started\r\n");
	if(get_process_priority(3) == 1){
			uart0_put_string("G006_test: test 1 OK\r\n");
					numTestPassed++;
	} else {
			uart0_put_string("G006_test: test 1 FAIL\r\n");
	}
			
	for (i = 0; i < 30; i++) {
		tmppt = request_memory_block();
		printf("proc3: tmppt = 0x%08x\r\n", tmppt);
	}
	if(tmppt == (void*)0x10006300){
			uart0_put_string("G006_test: test 2 OK\r\n");
			numTestPassed++;
	} else {
			uart0_put_string("G006_test: test 2 FAIL\r\n");
	}
	
	temp = set_process_priority(2, 0);
	
	if(gp_pcbs[1]->m_state == BLK){
			uart0_put_string("G006_test: test 3 OK\r\n");
					numTestPassed++;
	} else {
			uart0_put_string("G006_test: test 3 FAIL\r\n");
	}
	
	temp = release_memory_block(tmppt);
	
	if(temp == 0){
			uart0_put_string("G006_test: test 4 OK\r\n");
					numTestPassed++;
	} else {
			uart0_put_string("G006_test: test 4 FAIL\r\n");
	}
	printf("proc3 released mem %d\r\n", temp);
	//release_processor();
	
	switch(numTestPassed){
		case 0:
			uart0_put_string("G006_test: 0/6 OK\r\n");
			uart0_put_string("G006_test: 6/6 FAIL\r\n");
			break;
		case 1:
			uart0_put_string("G006_test: 1/6 OK\r\n");
			uart0_put_string("G006_test: 5/6 FAIL\r\n");
			break;
		case 2:
			uart0_put_string("G006_test: 2/6 OK\r\n");
			uart0_put_string("G006_test: 4/6 FAIL\r\n");
			break;
		case 3:
			uart0_put_string("G006_test: 3/6 OK\r\n");
			uart0_put_string("G006_test: 3/6 FAIL\r\n");
			break;
		case 4:
			uart0_put_string("G006_test: 4/6 OK\r\n");
			uart0_put_string("G006_test: 2/6 FAIL\r\n");
			break;
		case 5:
			uart0_put_string("G006_test: 5/6 OK\r\n");
			uart0_put_string("G006_test: 1/6 FAIL\r\n");
			break;
		case 6:
			uart0_put_string("G006_test: 6/6 OK\r\n");
			uart0_put_string("G006_test: 0/6 FAIL\r\n");
			break;
	}
	
	set_process_priority(3, 3);
	while(1){
		release_processor();
	}
}

void proc4(void)
{
	printf("proc4 started\r\n");
	
	set_process_priority(4, 3);
	while(1){
		release_processor();
	}
}

void proc5(void)
{
	printf("proc5 started\r\n");

	set_process_priority(5, 3);
	while(1){
		release_processor();
	}
}

void proc6(void)
{
	printf("proc6 started\r\n");
	

	set_process_priority(6, 3);
	while(1){
		release_processor();
	}
}