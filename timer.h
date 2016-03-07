/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#define timer_i_pcb gp_pcbs[10]

#include "k_rtx.h"

extern int timer_init ( int n_timer );  /* initialize timer n_timer */
extern int g_timer_count; // increment every 1 ms
extern int g_second_count; // increment every 1 ms
extern int terminated;
extern int string_to_time( char* time );

void timer_i_process(void);

#endif /* ! _TIMER_H_ */