#include "k_memory.h"
#include "k_process.h"

#ifdef DEBUG_0
#include "printf.h"

void k_send_message(uint32 recieving_pid, msg_t * env){
	//init message 
	env.dest_pid = recieving_pid;
	env.sender_pid = getCurrPCB().pid;
	// m.expir = 	 
	// m.type =	

	//add msg to to desination PCB linked list
	if(getPCB(recieving_pid)->first_msg == NULL){
		getPCB(recieving_pid)->first_msg = env;
		getPCB(recieving_pid)->last_msg = env;
	}
	else{
		msg_t temp* = getPCB(recieving_pid)->last_msg;
		temp->next = env;
		getPCB(recieving_pid)->last_msg = env;
	}
}

msg_t* k_receive_message(void){
	
}

void k_delayed_send(uint32, msg_t *, unint32){

}

#endif /* ! DEBUG_0 */