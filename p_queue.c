#include "p_queue.h"

void p_queue_init(p_queue* queue)
{
    queue->first = NULL;
    queue->last = NULL;
		queue->size = 0;
}

void p_enqueue(p_queue* queue, PCB* node_add)
{
    queue->size = queue->size + 1;
    if (queue->last != NULL){
        PCB* temp = queue->last;
        queue->last->next = node_add;
        node_add->prev = temp;
    }
    
    if(queue->first == NULL)
    {
        queue->first = node_add;
    }
    
    queue->last = node_add;
}

PCB* p_findproc(p_queue* queue){
    PCB* cur_node;
    PCB* prev;
    PCB* next;
    PCB* retNode;
    int count;
    
    cur_node = queue->first;
    
	count = 0;
    while(count < queue->size)
    {
			//This needs to be changed for later parts
        if (cur_node->m_state == RDY || cur_node->m_state == NEW)
        {
            retNode = cur_node;
            return retNode;
        }
        cur_node = cur_node->next;
				count++;
    }
    
    return NULL;
}

PCB* p_findAllproc(p_queue* queue){
    PCB* cur_node;
    PCB* prev;
    PCB* next;
    PCB* retNode;
    int count;
    
    cur_node = queue->first;
    
		count = 0;
    while(count < queue->size)
    {
			//This needs to be changed for later parts
        if (cur_node->m_state == RDY || cur_node->m_state == NEW || cur_node->m_state == RUN)
        {
            retNode = cur_node;
            return retNode;
        }
        cur_node = cur_node->next;
				count++;
    }
    
    return NULL;
}


PCB* p_findblockedproc(p_queue* queue){
    PCB* cur_node;
    PCB* prev;
    PCB* next;
    PCB* retNode;
    int count;
    
    cur_node = queue->first;
    
	count = 0;
    while(count < queue->size)
    {
			//This needs to be changed for later parts
			//Not need to add MSG_BLK
        if (cur_node->m_state == BLK)
        {
            retNode = cur_node;
            return retNode;
        }
        cur_node = cur_node->next;
				count++;
    }
    
    return NULL;
}

PCB* p_dequeue(p_queue* queue)
{
    PCB* ret_node;
    PCB* next_node;
    
    
    ret_node = queue->first;
    
    if (ret_node == NULL)
    {
        return NULL;
    }
    
    next_node = ret_node->next;
    queue->first = next_node;
    
    if (next_node == NULL)
    {
        queue->first = next_node;
        queue->last = next_node;
    }
    else
    {
        next_node->next = NULL;
    }
    
    return ret_node;
}

void p_queue_remove(p_queue* queue, int id)
{
		
    PCB* cur_node;
    PCB* prev;
    PCB* next;
    int count;
    
    cur_node = queue->first;
		queue->size = queue->size - 1;
	
		count = 0;
    while(count < queue->size)
    {
        if (cur_node->m_pid == id)
        {
            if(cur_node == queue->first){
								next = cur_node->next;
								next->prev = NULL;
                queue->first = next;
                return;
            }
            
            prev = cur_node->prev;
            prev->next = cur_node->next;
            
            next = cur_node->next;
            next->prev = cur_node->prev;
        }
        cur_node = cur_node->next;
				count++;
    }
}