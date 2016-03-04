#ifndef K_MESSAGE_H_
#define K_MESSAGE_H_

#include "k_rtx.h"
#include "p_queue.h"

#define DEFAULT 0
#define KCD_REG 1
#define CRT_DISPLAY 2



typedef struct msgbuf{

//#ifdef K_MSG_ENV
	void* mp_next;
	int m_send_pid;
	int m_recv_pid;
	int m_kdata[5];
//#endif
	int mtype;
	char mtext[1];
} msgbuf;

//Function declarations
int k_send_message(int , void *);
void* k_receive_message(int* );

int k_delayed_send(int, void *, int);

#endif

