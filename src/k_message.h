#ifndef K_MESSAGE_H_
#define K_MESSAGE_H_

#include "k_rtx.h"
#include "p_queue.h"

//Function declarations
void message_init(void);
void k_send_message(uint32, msg_t *);
msg_t* k_receive_message(void);
void k_delayed_send(uint32, msg_t *, unint32);

typedef struct msg_t{
	 void* next;
	 time_t expir;
	 unint32 sender_pid;
	 unint32 dest_pid;
	 unit8 msg_type;
} msg_t;


#endif

