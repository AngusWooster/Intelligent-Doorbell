#include "app_cfg.h"
#include "sys_paras.h"
#include "key.h"
#include "udp_client.h"
#include "pwr.h"
#include "led_ctrl.h"

//____ extern funt & var____
extern void sys_init(void);	// Init hw, os, vect ...
//__________________________

static system_paras sys_paras;






int main(void)
{
	sys_init();
	

	task_pwr_start((void *)&sys_paras);
	task_key_start((void *)&sys_paras);
	task_udpclient_start((void *)&sys_paras);
	task_led_ctrl_start((void *)&sys_paras);
	
	OSStart();

	return 0;
}

