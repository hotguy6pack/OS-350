
#ifndef SYS_PROC_H_
#define SYS_PROC_H_

extern int CRT_PROC_ID;
extern int KCD_PROC_ID;

void set_sys_procs(void);
void nullproc(void);
void kcd(void);
void crt(void);

#endif /* SYS_PROC_H_ */
