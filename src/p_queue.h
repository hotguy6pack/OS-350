#ifndef _P_QUEUE_H_
#define _P_QUEUE_H_

#include "k_process.h"


typedef struct p_queue{
    PCB* first;
    PCB* last;   
		int size;
} p_queue;

void node_init(PCB* node, int p_id);

void p_queue_remove(p_queue* queue, int id);

void p_queue_init(p_queue* queue);

void p_enqueue(p_queue* queue, PCB* node_add);

PCB* p_dequeue(p_queue* queue);


#endif
