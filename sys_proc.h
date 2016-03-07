
#ifndef SYS_PROC_H_
#define SYS_PROC_H_
#include "k_rtx.h"

extern int CRT_PROC_ID;
extern int KCD_PROC_ID;
extern int CLK_PROC_ID;
extern int PRIORITY_CHANGE_PROC_ID;
extern command_registry *command_head;
extern int command_registry_current_count;

void set_sys_procs(void);
void nullproc(void);
void kcd(void);
void crt(void);
void clock_proc(void);
void priority_change_proc(void);

#endif /* SYS_PROC_H_ */
