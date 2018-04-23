#include "common.h"
#include "hal_gpio.h"
#include "udp_client.h"
#include "led_ctrl.h"



extern udp_client_cmd_object* p_udp_cmd_ctrl;
extern led_ctrl_cmd_object* p_led_cmd_ctrl;



static void task_key(void* pdata)
{
	BOOL button_state = DISABLE;
	const system_paras *p_paras = (system_paras *)pdata;
	
	while(1) {
		sleep(30);

		if(!p_paras->battery_low) {
			if(gpio_hal_get_key_state()) {
				if (button_state == DISABLE) {
					button_state = ENABLE;
					printf("push upkey\n");
					p_led_cmd_ctrl->led_ctrl_set(LED_WHITE, LED_ON);
					p_udp_cmd_ctrl->udp_socket_set(ENABLE);
				}
			} else {
				if (button_state == ENABLE) {
					button_state = DISABLE;
					printf("release upkey\n");
					p_led_cmd_ctrl->led_ctrl_set(LED_WHITE, LED_OFF);
					p_udp_cmd_ctrl->udp_socket_set(DISABLE);
				}
			}
		}
	}
}


void task_key_start((void*) paras)
{
	thread_create(task_key, paras, TASK_KEY_PRIO, 0, TASK_KEY_STACK_SIZE, "task_key");
}

