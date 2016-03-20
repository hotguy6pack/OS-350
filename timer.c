/**
 * @brief timer.c - Timer example code. Tiemr IRQ is invoked every 1ms
 * @author T. Reidemeister
 * @author Y. Huang
 * @author NXP Semiconductors
 * @date 2012/02/12
 */

#include <LPC17xx.h>
#include "timer.h"
#include "k_message.h"
#include "k_memory.h"
#include "sys_proc.h"
#include "i_proc.h"


#define BIT(X) (1<<X)

int g_timer_count; // increment every 1 ms
int g_second_count;
int terminated;
int g_clock_display_force = 1;

msgbuf* timer_q;
int release_flag;


/**
 * @brief: initialize timer. Only timer 0 is supported
 */
int timer_init(int n_timer) 
{
	LPC_TIM_TypeDef *pTimer;
	if (n_timer == 0) {
		pTimer->PR = 12499;  
		pTimer->MR0 = 1;
		pTimer->MCR = BIT(0) | BIT(1);
		g_timer_count = 0;
		NVIC_EnableIRQ(TIMER0_IRQn);
		pTimer->TCR = 1;
		pTimer = (LPC_TIM_TypeDef *) LPC_TIM0;
	} else if ( n_timer == 1 ){
		pTimer = (LPC_TIM_TypeDef *) LCP_TIM1;
		pTimer->PR = 49;
		pTimer->MCR = BIT(0) | BIT(1);
		pTimer->TCR = BIT(0);
		pTimer->TC = 1;
		tc_count = &pTimer->TC;
	} else { 
		return 1;
	}
	
	timer_q = NULL;
	g_timer_count = 0;
	g_second_count = 0;
	terminated = 0;
	release_flag = 0;
	return 0;
}

// Value of sec in hh:mm:ss format
char* time_to_string(){
	int sec_to_display;
	int hour;
	int min;
	int sec;
	
	char temp;
	char buffer[10];
	int i;
	
	i = 0;
	
	sec_to_display = g_second_count;
	
	sec = sec_to_display % 60;
	min = (sec_to_display / 60) % 60;
	hour = (sec_to_display / 3600) % 24;
	
	temp = (hour / 10) + '0';
	buffer[i++] = temp;
	temp = (hour % 10) + '0';
	buffer[i++] = temp;
	
	buffer[i++] = ':';
	
	temp = (min / 10) + '0';
	buffer[i++] = temp;
	temp = (min % 10) + '0';
	buffer[i++] = temp;
	
	buffer[i++] = ':';
	
	temp = (sec / 10) + '0';
	buffer[i++] = temp;
	temp = (sec % 10) + '0';
	buffer[i++] = temp;
	
	buffer[i++] = '\r';
	buffer[i++] = '\n';
	//buffer[i++] = '\0';
	
	return buffer;
}

void update_clock(){
	msgbuf* msg;
	char *time;
	int debug = 0;
	
	if ( g_clock_display_force == 1 || (terminated == 0 && (debug == 1 || g_timer_count / 1000 > g_second_count)) ){
		g_clock_display_force = 0;
		g_second_count = g_timer_count / 1000; // convert from ms to s
		msg = (msgbuf *) k_request_memory_block_i();
		if (msg != NULL){
			msg->mtype = CRT_DISPLAY;
			time = time_to_string();
			strncpy(msg->mtext, time, strlen(time));
			k_send_message_i(CRT_PROC_ID, msg);
		}
	}
}

/**
 * @brief: use CMSIS ISR for TIMER0 IRQ Handler
 * NOTE: This example shows how to save/restore all registers rather than just
 *       those backed up by the exception stack frame. We add extra
 *       push and pop instructions in the assembly routine. 
 *       The actual c_TIMER0_IRQHandler does the rest of irq handling
 */
__asm void TIMER0_IRQHandler(void)
{
	CPSID I; //disable irq
	
	PRESERVE8
	IMPORT c_TIMER0_IRQHandler
	IMPORT k_release_processor
	
	PUSH{r4-r11, lr}
	BL c_TIMER0_IRQHandler
	LDR R4, = __cpp(&release_flag)
	LDR R4, [R4]
	CMP R4, #0

	CPSIE I; //enable irq

	BEQ POP_STACK
	BL k_release_processor

POP_STACK
	POP{r4-r11, pc}
} 
/**
 * @brief: c TIMER0 IRQ Handler
 */
void c_TIMER0_IRQHandler(void)
{
	
	/* ack inttrupt, see section  21.6.1 on pg 493 of LPC17XX_UM */
	LPC_TIM0->IR = BIT(0);  
	g_timer_count+= 30;
	update_clock();
	timer_i_process();
}

// Value of hh:mm:ss in sec
int string_to_time(char *time){
	int sec;
	int temp;
	
	sec = 0;
	
	// hh
	temp = time[0] - '0';
	temp *= 10;
	sec += temp * 3600;
	temp = time[1] - '0';
	sec += temp * 3600;
	
	// mm
	temp = time[3] - '0';
	temp *= 10;
	sec += temp * 60;
	temp *= time[4] - '0';
	sec += temp * 60;
	
	// ss
	sec += (time[6] - '0') * 10;
	sec += (time[6] - '0');
	
	return sec;
}

void timer_i_process(void) {
	msgbuf *cur_msg, *runner, *prev;
	int sender_id;
	PCB* orig_proc;
	
	release_flag = 0;
	orig_proc = gp_current_process;
	gp_current_process = timer_i_pcb;
	
	while(!is_message_empty(TIME_PROC_ID)) {
		cur_msg = (msgbuf*) k_receive_message(&sender_id);
		
		runner = timer_q;
		prev = NULL;
		
		while (runner != NULL && cur_msg->m_expiry > runner->m_expiry){
			prev = runner;
			runner = runner->mp_next;
		}
		
		if (prev != NULL){
			prev->mp_next = cur_msg;
			cur_msg->mp_next = runner;
		}else{
			cur_msg->mp_next = timer_q;
			timer_q = cur_msg;
		}
	}
	
	
	while (timer_q != NULL && timer_q->m_expiry <= g_timer_count) {
		cur_msg = timer_q;
		timer_q = cur_msg->mp_next;
		
		//send message to appropriate process
		gp_current_process = gp_pcbs[(cur_msg->m_send_pid) - 1];
		k_send_message_i(cur_msg->m_recv_pid, cur_msg);
		
		if (gp_pcbs[(cur_msg->m_recv_pid) - 1]->m_priority <= orig_proc->m_priority) {
			release_flag = 1;
		}
	}
	
	gp_current_process = orig_proc;
}
