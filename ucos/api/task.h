#ifndef _TASK_H_
#define _TASK_H_

void thread_create_init(void);
int thread_create(void(*task)(void *p_arg), void *p_arg, unsigned int prio, unsigned int *pbos, unsigned int stk_size, char *name);
int thread_exit(int thread_id);

#endif