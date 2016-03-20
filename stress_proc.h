/**
 * @file:   usr_proc.h
 * @brief:  Two user processes header file
 * @author: Yiqing Huang
 * @date:   2014/01/17
 */
 
#ifndef STRESS_PROC_H_
#define STRESS_PROC_H_

extern int PROC_A_ID;
extern int PROC_B_ID;
extern int PROC_C_ID;

void set_stress_procs(void);
void procA(void);
void procB(void);
void procC(void);

#endif /* STRESS_PROC_H_ */
