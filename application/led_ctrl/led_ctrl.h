#ifndef _LED_CTRL_H_
#define _LED_CTRL_H_


typedef enum{
	LED_OFF,
	LED_ON,
} LED_STATE;

typedef enum{
	LED_WHITE,
	LED_RED,
	LED_ALL
}LED_COLOR;

typedef struct{
	void (*led_ctrl_set)(LED_COLOR led, LED_STATE state);
	void (*led_ctrl_low_pwr_mode)(BOOL en);
}led_ctrl_cmd_object;

void task_led_ctrl_start((void*) paras);


#endif
