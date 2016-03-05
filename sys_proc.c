#include "rtx.h"
#include "uart_polling.h"
#include "sys_proc.h"
#include "k_process.h"
#include "k_message.h"
#ifdef DEBUG_0
#include "printf.h"

#endif /* DEBUG_0 */

PROC_INIT g_sys_procs[NUM_TEST_PROCS];

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

void kcd(void) {
	printf("kcd started\r\n");
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