
#include "i_proc.h"
#include "k_process.h"
#include "rtx.h"

PROC_INIT g_i_procs[NUM_I_PROCS];
int TIME_PROC_ID;
int UART_PROC_ID;

void set_i_procs() {
	int i;
	
	for( i = 0; i < NUM_I_PROCS; i++ ) {
		g_i_procs[i].m_pid=(U32)(i+NUM_TEST_PROCS+NUM_STRESS_PROCS+NUM_SYS_PROCS+1);
		g_i_procs[i].m_priority=NULLPROC;
		g_i_procs[i].m_stack_size=0x100;
	}
	
	//g_i_procs[0].mpf_start_pc = &timer_i_process;
	//g_i_procs[1].mpf_start_pc = &???????????????;
	
	UART_PROC_ID = NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + 1;
	TIME_PROC_ID = NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + 2;
}