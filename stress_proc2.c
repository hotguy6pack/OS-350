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
	
	g_test_procs[0].m_priority=HIGH;
	g_test_procs[1].m_priority=HIGH;
	g_test_procs[2].m_priority=HIGH;
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
	msgbuf* p;
	int sender_id;
	int num;
	
	p = (msgbuf*) request_memory_block();
	// register the %Z command
	p->mtype = KCD_REG;
	strcpy(p->mtext, "%Z");
	send_message(KCD_PROC_ID, p);
	// reg_cmd("Z", 1);
	
	while(1) {
		p = receive_message(&sender_id);
		if (strstr(p->mtext, "%Z") != NULL) { //checks if message text contains "%Z"
			release_memory_block(p);
			break;
		} else {
			release_memory_block(p);
		}
	}
	
	num = 0;
	
	while(1){
		p = (msgbuf*) request_memory_block();
		p->mtype = COUNT_REPORT;
		p->m_kdata[0] = num;
		//__enable_irq();////////////////////////////////////
		send_message(2, p);
		num = num + 1;
		release_processor();
	}
}

/**
 * @brief: a process that prints 5x6 numbers
 *         and then yields the cpu.
 */
void proc2(void)
{
	msgbuf* p;
	int sender_id;
	
	while(1){
		p = receive_message(&sender_id);
		send_message(3, p);
	}
}

void proc3(void)
{
	msgbuf* p;
	int sender_id;
	msgbuf* q;
	msgbuf* p2;
	msgbuf* msg_q;
	char* data;
	
	msgbuf* temp_msg_ptr;
	
	data = "Process C\r\n"; //////////////////////
	msg_q = NULL;
	
	while(1){
		if (msg_q == NULL) {
			p = receive_message(&sender_id);
		} else {
			// dequeue message from local queue
			p = msg_q;
			msg_q = (msgbuf*) msg_q->mp_next;
		}
		
		if (p->mtype == COUNT_REPORT) {
			if (p->m_kdata[0] % 20 == 0) {
				//send "Process C" to CRT display using msg envelope p
				p->mtype = CRT_DISPLAY;
				strcpy(p->mtext, data, strlen(data));
				send_message(CRT_PROC_ID, p);
			} else if (p->m_kdata[0] % 20 == 1) {
				//hibernate for 10 sec
				q = p;//(msgbuf*) request_memory_block();
				q->mtype = WAKEUP10;
				delayed_send(3, q, 50);
				while (1) {
					p2 = receive_message(&sender_id);
					if (p2->mtype == WAKEUP10) {
						break;
					} else {
						// put p into local queue
						temp_msg_ptr = msg_q;
						if(msg_q==NULL){
							msg_q=p2;
						}else{
							while (temp_msg_ptr->mp_next != NULL) {
								temp_msg_ptr = (msgbuf*) temp_msg_ptr->mp_next;
							}
							temp_msg_ptr->mp_next = (void*) p2;
						}
						p2->mp_next=NULL;
					}
				}
				release_memory_block(p);
			}else{
				release_memory_block(p);
			}
		}else{
			release_memory_block(p);
		}
		release_processor();
	}
}

void proc4(void)
{
	while(1){
		//printf("inside proc4\r\n");
		release_processor();
	}
}

void proc5(void)
{
	while(1){
		//printf("inside proc5\r\n");
		release_processor();
	}
}

void proc6(void)
{
	while(1){
		//printf("inside proc6\r\n");
		release_processor();
	}
}