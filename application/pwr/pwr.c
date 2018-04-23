#include "common.h"
#include "udp_client.h"
#include "led_ctrl.h"


extern udp_client_cmd_object* p_udp_cmd_ctrl;
extern led_ctrl_cmd_object* p_led_cmd_ctrl;

extern int pwr_detect(void);	// battery detect, volt >= 3.5 (TRUE) , volt < 3.5 (FALSE)


static void task_pwr(void* pdata)
{
	BOOL system_work = DISABLE;
	system_paras *p_paras = (system_paras *)pdata;
	while(1) {
		sleep(1000);
		
		if (pwr_detect()) {
			if (system_work == DISABLE) {
				p_paras->battery_low = FALSE;
				system_work = ENABLE;
				p_led_cmd_ctrl->led_ctrl_low_pwr_mode(FALSE);

			}
		} else {
			if (system_work == ENABLE) {
				p_paras->battery_low = TRUE;
				system_work = DISABLE;
				p_led_cmd_ctrl->led_ctrl_low_pwr_mode(TRUE);
			

			}

			if (p_udp_cmd_ctrl->udp_socket_state() == ENABLE) {
				p_udp_cmd_ctrl->udp_socket_set(DISABLE);
			}
		}
	}
}



void task_pwr_start((void*) paras)
{
	thread_create(task_pwr, paras, TASK_PWR_PRIO, 0, TASK_PWR_STACK_SIZE, "task_pwr");
}

