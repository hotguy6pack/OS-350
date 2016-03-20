#include "k_memory.h"
#include "k_process.h"
#include "k_message.h"
#include "timer.h"

#ifdef DEBUG_0
#include "printf.h"

int k_send_message(int process_id, void * message_envelope){
	
	//init message 
	msgbuf* message = (msgbuf*) message_envelope;
	#ifdef _OBJ
				process_id = process_map(process_id);
	#endif
	message->m_recv_pid = process_id;
	message->m_send_pid = gp_current_process->m_pid;

	if(NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + NUM_I_PROCS < process_id - 1){
		return 1;
	}
	else if(process_id < 0){
		return 1;
  }
	// m.type =	

	//add msg to to desination PCB linked list
	
	if(gp_pcbs[process_id-1]->first_msg == NULL){
		gp_pcbs[process_id-1]->first_msg = message;
		gp_pcbs[process_id-1]->last_msg = message;
	}
	else{
		msgbuf* temp = gp_pcbs[process_id-1]->last_msg;
		temp->mp_next = message;
		gp_pcbs[process_id-1]->last_msg = message;
	}
	
	if(gp_pcbs[process_id-1]->m_state == MSG_BLK){

			gp_pcbs[process_id-1]->m_state = RDY;
		
			if(gp_pcbs[process_id-1]->m_priority <= gp_current_process->m_priority){
					k_release_processor();
			}
	}

	return 0;
}

int k_send_message_i(int process_id, void * message_envelope){
	//init message 
	msgbuf* message = (msgbuf*) message_envelope;
	
	message->m_recv_pid = process_id;
	
	if(NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + NUM_I_PROCS < process_id - 1){
		return 1;
	}
	else if(process_id < 0){
		return 1;
  }
	// m.type =	

	//add msg to to desination PCB linked list
	
	if(gp_pcbs[process_id-1]->first_msg == NULL){
		gp_pcbs[process_id-1]->first_msg = message;
		gp_pcbs[process_id-1]->last_msg = message;
	}
	else{
		msgbuf* temp = gp_pcbs[process_id-1]->last_msg;
		temp->mp_next = message;
		gp_pcbs[process_id-1]->last_msg = message;
	}
	
	if(gp_pcbs[process_id-1]->m_state == MSG_BLK){

			gp_pcbs[process_id-1]->m_state = RDY;
		
			//if(gp_pcbs[process_id-1]->m_priority <= gp_current_process->m_priority){
			//		k_release_processor();
			//}
	}

	return 0;
}



void* k_receive_message(int * sender_id){
	
	msgbuf* envo;
	while(gp_current_process->first_msg==NULL){
		//printf("BLOCK HERE\r\n");
		gp_current_process->m_state = MSG_BLK;
		k_release_processor();
	}
	envo = gp_current_process->first_msg;
	if(gp_current_process->first_msg != gp_current_process->last_msg){
		gp_current_process->first_msg = envo->mp_next;
	}else{
		gp_current_process->first_msg=NULL;
		gp_current_process->last_msg=NULL;
	}
	
	*sender_id = envo->m_send_pid;
	return envo;
}

int k_delayed_send(int process_id, void * message_envelope, int delay){
		//init message 
	
	msgbuf* message = (msgbuf*) message_envelope;
	
	#ifdef _OBJ
				process_id = process_map(process_id);
	#endif
	
	message->m_recv_pid = process_id;
	message->m_send_pid = gp_current_process->m_pid;
	message->m_expiry = g_timer_count + delay;

	if(delay==0){
		 return k_send_message(process_id, message_envelope);
	}
	
	if(NUM_TEST_PROCS + NUM_STRESS_PROCS + NUM_SYS_PROCS + NUM_I_PROCS < process_id - 1){
		return 1;
	}
	else if(process_id < 0){
		return 1;
  }
	
	//add msg to to desination PCB linked list
	
	if(timer_i_pcb->first_msg == NULL){
		timer_i_pcb->first_msg = message;
		timer_i_pcb->last_msg = message;
	}
	else{
		msgbuf* temp = timer_i_pcb->last_msg;
		temp->mp_next = message;
		timer_i_pcb->last_msg = message;
	}

	return 0;
}

int is_message_empty(int curr_PID){
	if(gp_pcbs[curr_PID-1]->first_msg==NULL){
		return 1;
	}
	return 0;
}

int is_timer_message_empty(){
	if(timer_i_pcb->first_msg==NULL){
		return 1;
	}
	return 0;
}

#endif /* ! DEBUG_0 */