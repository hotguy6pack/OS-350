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
int starved_clock;


/**
 * @brief: initialize timer. Only timer 0 is supported
 */
int timer_init(int n_timer) 
{
	LPC_TIM_TypeDef *pTimer;
	if (n_timer == 0) {
		/*
		Steps 1 & 2: system control configuration.
		Under CMSIS, system_LPC17xx.c does these two steps
		
		----------------------------------------------------- 
		Step 1: Power control configuration.
		        See table 46 pg63 in LPC17xx_UM
		-----------------------------------------------------
		Enable UART0 power, this is the default setting
		done in system_LPC17xx.c under CMSIS.
		Enclose the code for your refrence
		//LPC_SC->PCONP |= BIT(1);
	
		-----------------------------------------------------
		Step2: Select the clock source, 
		       default PCLK=CCLK/4 , where CCLK = 100MHZ.
		       See tables 40 & 42 on pg56-57 in LPC17xx_UM.
		-----------------------------------------------------
		Check the PLL0 configuration to see how XTAL=12.0MHZ 
		gets to CCLK=100MHZ in system_LPC17xx.c file.
		PCLK = CCLK/4, default setting in system_LPC17xx.c.
		Enclose the code for your reference
		//LPC_SC->PCLKSEL0 &= ~(BIT(3)|BIT(2));	

		-----------------------------------------------------
		Step 3: Pin Ctrl Block configuration. 
		        Optional, not used in this example
		        See Table 82 on pg110 in LPC17xx_UM 
		-----------------------------------------------------
		*/
		pTimer = (LPC_TIM_TypeDef *) LPC_TIM0;

	} else { /* other timer not supported yet */
		return 1;
	}

	/*
	-----------------------------------------------------
	Step 4: Interrupts configuration
	-----------------------------------------------------
	*/

	/* Step 4.1: Prescale Register PR setting 
	   CCLK = 100 MHZ, PCLK = CCLK/4 = 25 MHZ
	   2*(12499 + 1)*(1/25) * 10^(-6) s = 10^(-3) s = 1 ms
	   TC (Timer Counter) toggles b/w 0 and 1 every 12500 PCLKs
	   see MR setting below 
	*/
	pTimer->PR = 12499;  

	/* Step 4.2: MR setting, see section 21.6.7 on pg496 of LPC17xx_UM. */
	pTimer->MR0 = 1;

	/* Step 4.3: MCR setting, see table 429 on pg496 of LPC17xx_UM.
	   Interrupt on MR0: when MR0 mathches the value in the TC, 
	                     generate an interrupt.
	   Reset on MR0: Reset TC if MR0 mathches it.
	*/
	pTimer->MCR = BIT(0) | BIT(1);

	g_timer_count = 0;

	/* Step 4.4: CSMSIS enable timer0 IRQ */
	NVIC_EnableIRQ(TIMER0_IRQn);

	/* Step 4.5: Enable the TCR. See table 427 on pg494 of LPC17xx_UM. */
	pTimer->TCR = 1;
/*
	timer_i_pcb->mp_sp = NULL;	
	timer_i_pcb->m_priority = 0; 
	timer_i_pcb->m_pid = 0;		
	timer_i_pcb->m_state = RUN;   
	timer_i_pcb->m_pc = NULL; 
	timer_i_pcb->next = NULL; 
	timer_i_pcb->prev = NULL; 
	timer_i_pcb->first_msg = NULL;
	timer_i_pcb->last_msg = NULL;
	*/
	timer_q = NULL;
	g_timer_count = 0;
	g_second_count = 0;
	terminated = 0;
	release_flag = 0;
	starved_clock = 0;

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
	
	if(starved_clock == 1){
		msg = (msgbuf *) k_request_memory_block_i();
		if (msg != NULL){
			msg->mtype = CRT_DISPLAY;
			time = time_to_string();
			strncpy(msg->mtext, time, strlen(time));
			k_send_message_i(CRT_PROC_ID, msg);
			starved_clock = 0;
		}
	}
	
	if ( g_clock_display_force == 1 || (terminated == 0 && (debug == 1 || g_timer_count / 1000 > g_second_count)) ){
		g_clock_display_force = 0;
		g_second_count = g_timer_count / 1000; // convert from ms to s
		msg = (msgbuf *) k_request_memory_block_i();
		if (msg != NULL){
			msg->mtype = CRT_DISPLAY;
			time = time_to_string();
			strncpy(msg->mtext, time, strlen(time));
			k_send_message_i(CRT_PROC_ID, msg);
			starved_clock = 0;
		}
		else{
			starved_clock = 1;
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
