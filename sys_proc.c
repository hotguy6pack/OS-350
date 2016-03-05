#include "rtx.h"
#include "uart_polling.h"
#include "sys_proc.h"
#include "k_process.h"
#include "k_message.h"
#ifdef DEBUG_0
#include "printf.h"

#endif /* DEBUG_0 */

PROC_INIT g_sys_procs[NUM_TEST_PROCS];
extern int CRT_PROC_ID;
extern int KCD_PROC_ID;

typedef enum {
	clk_reset,
	clk_set,
	clk_terminate,
	pri_set,
	invalid
} Commands;

void set_sys_procs() {
	int i;
	
	for( i = 0; i < NUM_SYS_PROCS; i++ ) {
		g_sys_procs[i].m_pid=(U32)(i+1);
		g_sys_procs[i].m_priority=NULLPROC;
		g_sys_procs[i].m_stack_size=0x100;
	}
	
	g_sys_procs[0].mpf_start_pc = &nullproc;
	g_sys_procs[1].mpf_start_pc = &kcd;
	g_sys_procs[2].mpf_start_pc = &crt;
	
	g_sys_procs[1].m_priority=HIGH;
	g_sys_procs[2].m_priority=HIGH;
}

void nullproc(void) {
	printf("nullproc started\r\n");
	while(1) {
		release_processor();
	}
}

Commands decode(char data[]){
	switch (data[1]){
		case 'W':
			switch (data[2]){
				case 'R':
					return clk_reset;
				case 'S':
					return clk_set;
				case 'T':
					return clk_terminate;
				default:
					return invalid;
			}
		case 'C':
			return pri_set;
		default:
			return invalid;
	}
}

void kcd(void) {
	int sender_id;
	char data[5];
	msgbuf* env;
	
	printf("kcd started\r\n");

	env = (msgbuf *)request_memory_block();
	env = receive_message(&sender_id);
	strncpy(data, env->mtext, 5);
	
	switch (decode(data)){
		case clk_reset:
			printf("Command - Reset Clock\r\n");
			break;
		case clk_set:
			printf("Command - Set Clock\r\n");
			break;
		case clk_terminate:
			printf("Command - Terminate Clock\r\n");
			break;
		case pri_set:
			printf("Command - Set Priority\r\n");
			break;
		default:
			// invalid command
			printf("Command - ERROR\r\n");
	}
	
	while(1) {
		release_processor();
	}
}

void crt(void) {
	printf("crt started\r\n");
	while(1) {
		release_processor();
	}
}