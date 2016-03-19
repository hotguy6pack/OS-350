#include "rtx.h"
#include "uart_polling.h"
#include "sys_proc.h"
#include "k_process.h"
#include "k_message.h"
#include "timer.h"
#include "i_proc.h"
#include <LPC17xx.h>
#include <string.h>
#include "k_rtx.h"
#ifdef DEBUG_0
#include "printf.h"

#endif /* DEBUG_0 */

PROC_INIT g_sys_procs[NUM_SYS_PROCS];
int CRT_PROC_ID;
int KCD_PROC_ID;
int CLK_PROC_ID;
int PRIORITY_CHANGE_PROC_ID;
int current_sys_proc_count;
command_registry *command_head;
int command_registry_current_count;

int substring_toi(char* s, int32_t n) {
    int base  = 1;
    int value  = 0;

    while (--n >= 0) {
        value += base * (s[n] - '0');
        base  *= 10;
    }

    return value;
}

void insert_to_head(command_registry* head, char * val, int proc_id){
	command_registry* lastNode;
	
	lastNode = (command_registry*) head + COMMAND_REG_SIZE * command_registry_current_count;
	
	if (exists(head, val) == 0){
		command_registry* newNode;
		command_registry_current_count++;
		newNode = (command_registry*) head + COMMAND_REG_SIZE * command_registry_current_count;
		newNode->next = NULL;
		newNode->val = val;
		newNode->proc_id = proc_id;
		lastNode->next = newNode;
	}
}

void set_sys_procs() {
	int i;
	
	current_sys_proc_count = 2;
	command_registry_current_count = 0;
	
	for( i = 0; i < NUM_SYS_PROCS; i++ ) {
		g_sys_procs[i].m_pid=(U32)(i+NUM_TEST_PROCS+1);
		g_sys_procs[i].m_priority=NULLPROC;
		g_sys_procs[i].m_stack_size=0x100;
	}
	
	g_sys_procs[0].mpf_start_pc = &nullproc;
	g_sys_procs[1].mpf_start_pc = &kcd;
	g_sys_procs[2].mpf_start_pc = &crt;
	g_sys_procs[3].mpf_start_pc = &clock_proc;
	
	g_sys_procs[1].m_priority=HIGH;
	KCD_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count++;
	g_sys_procs[2].m_priority=HIGH;
	CRT_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count++;
	g_sys_procs[3].m_priority=HIGH;
	CLK_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count++;
	
	command_head->val = "WR";
	command_head->next = NULL;
	command_head->proc_id = CLK_PROC_ID;
	
	reg_cmd("WS", CLK_PROC_ID);
	reg_cmd("WT", CLK_PROC_ID);
}

void reg_cmd(char* val, int proc_id){
	insert_to_head(command_head, val, proc_id);
}

int exists(command_registry* head, char * val){
	command_registry* current;
	
	current = head;
	
	while (current != NULL){
		if (strcmp(current->val,val) == 0){ 
			return current->proc_id;
		}
		current = current->next;
	}
	
	return 0;
}

int get_proc_id(command_registry* head, char * val){
	int id;
	command_registry* current;
	
	id = exists(head, val);
	
	if ( id != 0 ){
		return id;
	}
	
	return -1;
}

void nullproc(void) {
	printf ("nullproc started\r\n");
	while(1) {
		release_processor();
	}
}

void clock_proc(void){
	int sender_id;
	int i;
	msgbuf* env;
	msgbuf* env1;
	char *data;
	char *code;
	const char delim[2] = " ";
	char *token;
	int temp;
	const int message_size = 15;
	char message[message_size];
	
	printf ("clock process started\r\n");
	
	while(1) {
		env = receive_message(&sender_id);
		
		for (i = 0; i < message_size; ++i){
			message[i] = '\0';
		}
		
		if (sender_id == CLK_PROC_ID && terminated == 0){
			env->mtype = DEFAULT;
			// TODO: send delayed message by 1000
			k_delayed_send(CLK_PROC_ID, env, 1000);
			
			env1 = (msgbuf*) request_memory_block();
			env1->mtype = CRT_DISPLAY;
			sprintf(message, "\033[s\033[1;69H%02d:%02d:%02d\n\033[u", (g_second_count / 3600) % 24, (g_second_count / 60) % 60, (g_second_count % 60));
			strncpy(env1->mtext, message, strlen(message));
			
			
			g_second_count++;
			g_second_count = g_second_count % (60 * 60 * 24);
			
		}else{
			if(env->mtext[0] == '%' && env->mtext[1] == 'W' && env->mtext[2] == 'R'){
				printf("Command - Reset Clock\r\n");
				g_second_count = 0;
				g_timer_count = 0;
				g_clock_display_force = 1;
				terminated = 0;

			}else if (env->mtext[0] == '%' && env->mtext[1] == 'W' && env->mtext[2] == 'S'){ // TODO: Add strlen check
				int leng = strlen(env->mtext);
				
				printf("Command - Set Clock\r\n");

				g_second_count = 0;
				
				temp = substring_toi(&env->mtext[4], 2);
				g_second_count += temp *3600;
				
				temp = substring_toi(&env->mtext[7], 2) *60;
				g_second_count += temp ;
				
				temp = substring_toi(&env->mtext[10], 2);
				g_second_count += temp;
				
				g_clock_display_force = 1;
				g_timer_count = g_second_count * 1000;
				
				terminated = 0;
				
				if (env->mtext[14] != '\0' || env->mtext[6] != ':' || env->mtext[9] != ':'){
					g_second_count = 0;
					g_timer_count = 0;
				}
				
			}else if (env->mtext[0] == '%' && env->mtext[1] == 'W' && env->mtext[2] == 'T'){
				printf("Command - Terminate Clock\r\n");
				terminated = 1;
				send_message(CRT_PROC_ID, env);
			}
		}
	}
}

void kcd(void) {
	int sender_id;
	int receiver_id;
	int i;
	char *data;
	char *time;
	const char delim[2] = " ";
	char *token;
	
	msgbuf* env;
	
	printf("kcd started\r\n");

	while(1) {
		env = receive_message(&sender_id);
		token = strtok(env->mtext, delim);
		
		if (env->mtype == KCD_REG){
			insert_to_head( command_head, &token[1], env->m_send_pid );
			release_memory_block(env);
		}else{
			receiver_id = get_proc_id( command_head, &token[1] );
			env->mtype = DEFAULT;
			k_send_message_i(receiver_id, env);
			env->mtype = CRT_DISPLAY;
			k_send_message_i(CRT_PROC_ID, env);
		}
	}
}

void crt(void) {
	msgbuf* env;
	int sender_id;
	char data[MEM_BLK_SZ - 0x28];
	LPC_UART_TypeDef *pUart;
	
	printf("crt started\r\n");
	while(1) {
		env = receive_message(&sender_id);
		if (env->mtype == CRT_DISPLAY) {
			send_message(UART_PROC_ID, env);
			pUart = (LPC_UART_TypeDef *) LPC_UART0;
			pUart->IER = IER_THRE | IER_RLS | IER_RBR;
		}else{
			release_memory_block(env);	
		}
	}
}