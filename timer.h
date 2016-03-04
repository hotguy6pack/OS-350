/**
 * @brief timer.h - Timer header file
 * @author Y. Huang
 * @date 2013/02/12
 */
#ifndef _TIMER_H_
#define _TIMER_H_

#include "k_rtx.h"

extern int timer_init ( int n_timer );  /* initialize timer n_timer */
extern PCB *timer_i_pcb;

#endif /* ! _TIMER_H_ */