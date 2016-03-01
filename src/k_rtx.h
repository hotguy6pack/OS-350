/** 
 * @file:   k_rtx.h
 * @brief:  kernel deinitiation and data structure header file
 * @auther: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef K_RTX_H_
#define K_RTX_H_

/*----- Definitations -----*/

#define HIGH    0
#define MEDIUM  1
#define LOW     2
#define LOWEST  3

#define NULL 0
#define NUM_TEST_PROCS 7
#define NUM_MEM_BLKS 30

#define RTX_ERR -1
#define RTX_OK  0

#define RAM_TOP 0x10008000

#define MEM_BLK_SZ 0x80
#define PCB_SZ 0x40

#ifdef DEBUG_0
#define USR_SZ_STACK 0x200         /* user proc stack size 512B   */
#else
#define USR_SZ_STACK 0x100         /* user proc stack size 218B  */
#endif /* DEBUG_0 */

/*----- Types -----*/
typedef unsigned char U8;
typedef unsigned int U32;

/* process states, note we only assume three states in this example */

typedef enum {NEW = 0, RDY, RUN, BLK} PROC_STATE_E;  

/*
  PCB data structure definition.
  You may want to add your own member variables
  in order to finish P1 and the entire project 
*/
typedef struct PCB 
{ 
	//struct pcb *mp_next;  /* next pcb, not used in this example */  
	void* mp_sp;		/* stack pointer of the process (4 Bytes) */
	U8 m_priority; /*priotrity (1 Byte)*/
	U8 m_pid;		/* process id (1 Byte)*/
	PROC_STATE_E m_state;   /* state of the process (4 Bytes)*/
	void* m_pc; /* PC (4 Bytes) */
	struct PCB* next; /* 4 Bytes */
	struct PCB* prev; /* 4 Bytes */
} PCB;

/* initialization table item */
typedef struct proc_init2
{	
	int m_pid;	        /* process id */ 
	int m_priority;         /* initial priority, not used in this example. */ 
	int m_stack_size;       /* size of stack in words */
	void (*mpf_start_pc) ();/* entry point of the process */    
} PROC_INIT2;

#endif // ! K_RTX_H_
