#include "rtx.h"
#include "uart_polling.h"
#include "sys_proc.h"
#include "k_process.h"
#include "k_message.h"
#include "timer.h"
#include "i_proc.h"
#include <LPC17xx.h>
#include <string.h>
#ifdef DEBUG_0
#include "printf.h"

#endif /* DEBUG_0 */


typedef struct command_registry{
	char* val;
	int proc_id;
	struct command_registry* next;
} command_registry;

PROC_INIT g_sys_procs[NUM_SYS_PROCS];
int CRT_PROC_ID;
int KCD_PROC_ID;
int current_sys_proc_count;
command_registry *command_head;

void insert_to_head(command_registry* head, char * val, int proc_id){
	if (exists(head, val) == 0){
		command_registry* newNode;
		newNode = (command_registry*) request_memory_block();
		newNode->next = head;
		newNode->proc_id = proc_id;
		head = newNode;
	}
}

void set_sys_procs() {
	int i;
	
	current_sys_proc_count = 1;
	
	for( i = 0; i < NUM_SYS_PROCS; i++ ) {
		g_sys_procs[i].m_pid=(U32)(i+NUM_TEST_PROCS+1);
		g_sys_procs[i].m_priority=NULLPROC;
		g_sys_procs[i].m_stack_size=0x100;
	}
	
	g_sys_procs[0].mpf_start_pc = &nullproc;
	g_sys_procs[1].mpf_start_pc = &kcd;
	g_sys_procs[2].mpf_start_pc = &crt;
	
	g_sys_procs[1].m_priority=HIGH;
	KCD_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count++;
	g_sys_procs[2].m_priority=HIGH;
	CRT_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count++;
	
	command_head = (command_registry *)request_memory_block();
	command_head->val = "WR";
	command_head->next = NULL;
	command_head->proc_id = NUM_TEST_PROCS + current_sys_proc_count;
	
	insert_to_head(command_head, "WS", NUM_TEST_PROCS + current_sys_proc_count);
	insert_to_head(command_head, "WT", NUM_TEST_PROCS + current_sys_proc_count);
	
	current_sys_proc_count++;
	insert_to_head(command_head, "C", NUM_TEST_PROCS + current_sys_proc_count);
	
}

int exists(command_registry* head, char * val){
	command_registry* current;
	
	current = head;
	
	while (current != NULL){
		if (current->val == val){ 
			return 1;
		}
		current = current->next;
	}
	
	return 0;
}

int get_proc_id(command_registry* head, char * val){
	command_registry* current;
	
	if (exists(head, val) == 1){
		current = head;
		while (current != NULL){
			if (current->val == val){
				return current->proc_id;
			}
			current = current->next;
		}
	}
	
	return -1;
}

void nullproc(void) {
	printf ("nullproc started\r\n");
	while(1) {
		release_processor();
	}
}

void clock_proc(){
	
	int sender_id;
	msgbuf* env;
	char *data;
	char *code;
	
	env = receive_message(&sender_id);
	strncpy(data, env->mtext, strlen(env->mtext));
	code = &data[1]; // get the code minus the % character
	
	if(code == "WR"){
		printf("Command - Reset Clock\r\n");
		g_second_count = 0;
		g_timer_count = 0;
		terminated = 0;
	}else if (code == "WS"){
		printf("Command - Set Clock\r\n");
		g_second_count = string_to_time(&data[4]);
		g_timer_count = g_second_count * 1000;
		terminated = 0;
	}else if (code == "WT"){
		printf("Command - Terminate Clock\r\n");
		terminated = 1;
	}
	
	printf ("clock process started\r\n");
	
	while(1) {
		release_processor();
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
	msgbuf* env_send;
	
	printf("kcd started\r\n");

	while(1) {
		env = receive_message(&sender_id);
		strncpy(data, env->mtext, strlen(env->mtext));
		token = strtok(data, delim);
		
		if (env->mtype == KCD_REG){
			insert_to_head( command_head, &token[1], env->m_send_pid );
		}else{
			receiver_id = get_proc_id( command_head, &token[1] );
			env_send = (msgbuf *) request_memory_block();
			env_send->mtype = DEFAULT;
			strncpy(env_send->mtext, data, strlen(data));
			k_send_message_i(receiver_id, env_send);
		}
		release_memory_block(env);
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