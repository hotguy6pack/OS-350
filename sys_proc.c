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
	g_sys_procs[4].mpf_start_pc = &priority_change_proc;
	
	g_sys_procs[1].m_priority=HIGH;
	KCD_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count++;
	g_sys_procs[2].m_priority=HIGH;
	CRT_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count++;
	g_sys_procs[3].m_priority=HIGH;
	CLK_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count;
	g_sys_procs[4].m_priority=HIGH;
	PRIORITY_CHANGE_PROC_ID = NUM_TEST_PROCS + current_sys_proc_count;
	
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

void clock_proc(){
	int sender_id;
	msgbuf* env;
	char *data;
	char *code;
	const char delim[2] = " ";
	char *token;
	char *wr = "WR";
	char *ws = "WS";
	char *wt = "WT";
	
	printf ("clock process started\r\n");

	env = receive_message(&sender_id);
	token = strtok(env->mtext, delim);
	code = &token[1]; // get the code minus the % character

	if(strcmp(code, wr) == 0){
		printf("Command - Reset Clock\r\n");
		g_second_count = 0;
		g_timer_count = 0;
		terminated = 0;
	}else if (strcmp(code, ws) == 0){
		printf("Command - Set Clock\r\n");
		g_second_count = string_to_time(&data[4]);
		g_timer_count = g_second_count * 1000;
		terminated = 0;
	}else if (strcmp(code, wt) == 0){
		printf("Command - Terminate Clock\r\n");
		terminated = 1;
	}
	release_memory_block(env);
	
	while(1) {
		release_processor();
	}
}

void priority_change_proc(){
	
	int sender_id;
	msgbuf* env;
	char *data;
	char *code;
	int proc_id;
	int priority;
  const int message_size = 10;
	char message[message_size];
	int i = 0;
	
	printf ("priority change process started\r\n");
	
	// TODO: set priority for proc with id = proc_id to the new priority
	while(1) {
		env = receive_message(&sender_id);
		
		for (i = 0; i < message_size; ++i){
			message[i] = '/0';
		}

		strncpy(message, env->mtext, message_size);
		release_memory_block(env);
		
		if (strlen(message) < strlen("%C ") || message[2] != ' '){
			// ERROR
			return;
		}
		
		if (strlen(message) == strlen("%C 00 0\r\n")) {
			proc_id = substring_toi(&message[3], 2);
      priority = substring_toi(&message[6], 1);
		}else if (strlen(message) == strlen("%C 0 0\r\n")){
			proc_id = substring_toi(&message[3], 1);
      priority = substring_toi(&message[5], 1);
		}
		
		if (proc_id >= 1 && proc_id <= 13 && priority >= 0 && priority <= 3){
			// TODO: set priority to proc_id, priority
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
		}else{
			receiver_id = get_proc_id( command_head, &token[1] );
			env->mtype = DEFAULT;
			k_send_message_i(receiver_id, env);
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