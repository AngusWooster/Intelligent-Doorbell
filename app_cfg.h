#ifndef _APP_CFG_H_
#define _APP_CFG_H_


enum TASK_PRIO{
	TASK_PWR_PRIO,
	TASK_KEY_PRIO,
	TASK_UDPCLIENT_PRIO,
	TASK_LEDCTRL_PRIO
}

#define TASK_PWR_STACK_SIZE			512
#define TASK_KEY_STACK_SIZE			512
#define TASK_UDPCLIENT_STACK_SIZE	512
#define TASK_LEDCTRL_STACK_SIZE		512

#endif
