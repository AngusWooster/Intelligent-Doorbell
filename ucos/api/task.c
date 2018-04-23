#include "os_api.h"
#include "task.h"

unsigned int os_stack[OS_LOWEST_PRIO + 1];

void thread_create_init(void)
{
	memset(os_stack, 0, sizeof(os_stack));
}

int thread_create(void(*task)(void *p_arg), void *p_arg, unsigned int prio, unsigned int *pbos, unsigned int stk_size, char *name)
{
	INT8U ret, need_free_stack = 0;
	OS_TCB tcb;
	if (OSTaskQuery(prio, &tcb) == 0) {
		return -1;
	}
	if (pbos == 0) {
		pbos = (unsigned int *)mem_calloc(stk_size, sizeof(unsigned int));
		if (!pbos)
			return OS_ERR;
		os_stack[prio] = pbos;
	}

	ret = OSTaskCreateExt(task, p_arg, (OS_STK *)&pbos[stk_size - 1], prio, prio, (OS_STK *)pbos, stk_size, NULL, (INT16U)(OS_TASK_OPT_STK_CHK | OS_TASK_OPT_STK_CLR), need_free_stack);
	if (ret == OS_ERR_NONE) {
		return prio;
	} else {
		if (need_free_stack && pbos)
			mem_free(pbos);
	}
	return -1;
}

int thread_exit(int thread_id)
{
	int ret;
	ret = OSTaskDel(thread_id);
	return ret;
}
