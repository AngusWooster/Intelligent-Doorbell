#include "common.h"
#include "led_ctrl.h"
#include "hal_gpio.h"

#define LED_CTRL_PERIOD			25	//ms
#define LED_BLINK_PERIOD		500 //ms
#define LED_DUTY_HI_PERIOD		125 //(LED_CTRL_PERIOD*25/100) //ms
#define LED_DUTY_LOW_PERIOD		375 //(LED_BLINK_PERIOD - LED_DUTY_HI_PERIOD)	//ms


//________static api ________
static void led_ctrl_set(LED_COLOR led, LED_STATE state);
static void led_ctrl_low_pwr_mode(BOOL en);
//___________________________
typedef enum{
	evt_null,
	evt_led_noraml_mode,
	evt_led_setting_mode,
	evt_led_low_pwr_mode,	
}led_ctrl_event;

static led_ctrl_event led_ctrl_evt = evt_null;	
mbox_t mbox_led_ctrl_evt = NULL;

static const led_ctrl_cmd_object led_cmd_obj = {
	led_ctrl_set,
	led_ctrl_low_pwr_mode,
};

const led_ctrl_cmd_object* p_led_cmd_ctrl = &led_cmd_obj;




struct led_info {
	LED_STATE state;
};

static struct led_info led_light[LED_ALL];


static void led_ctrl_set(LED_COLOR led, LED_STATE state)
{
	if (mbox_led_ctrl_evt != NULL) {
		switch (led) {
		case LED_WHITE:
			led_light[LED_WHITE].state = state;
			led_ctrl_evt = evt_led_setting_mode;
			mbox_post(&mbox_led_ctrl_evt, (void *)&led_ctrl_evt);
			break;
		case LED_RED:
			//This led only can be set by low battery status
			break;
		default:
			//this color doesn't support
			break;
		}
	}
}

static void led_ctrl_low_pwr_mode(BOOL en)
{
	led_ctrl_evt = (en == ENABLE) ? evt_led_low_pwr_mode : evt_led_noraml_mode;

	if (led_ctrl_evt == evt_led_noraml_mode) {
		led_light[LED_WHITE].state = LED_OFF;
		led_light[LED_RED].state = LED_OFF;
	} else {
		led_light[LED_WHITE].state = LED_OFF;
		led_light[LED_RED].state = LED_ON;
	}
	mbox_post(&mbox_led_ctrl_evt, (void *)&led_ctrl_evt);
}

static void led_ctrl_lighting(void)
{
	if (led_light[LED_WHITE].state == LED_ON) {
		gpio_hal_set_white_led(ENABLE);
	} else {
		gpio_hal_set_white_led(DISABLE);
	}

	if (led_light[LED_RED].state == LED_ON) {
		gpio_hal_set_red_led(ENABLE);
	} else {
		gpio_hal_set_red_led(DISABLE);
	}
}

static void task_led_ctrl(void* pdata)
{	
	unsigned int t;
	const system_paras *p_paras = (system_paras*)pdata;
	memset(led_light, 0, sizeof(led_light));
	mbox_new(&mbox_led_ctrl_evt, (void *)0);
	led_ctrl_event* p_evt;
	BOOL led_low_pwr_mode = FALSE;
	while(1) {
		p_evt = mbox_get(&mbox_led_ctrl_evt, LED_CTRL_PERIOD);	

		if (p_evt != NULL) {
			led_low_pwr_mode = (*p_evt == evt_led_low_pwr_mode) ? TRUE : FALSE;
			led_ctrl_lighting();
			if (led_low_pwr_mode) {
				t = os_time_get();
			}
		}

		if (led_low_pwr_mode) {
			if (led_light[LED_RED].state == LED_ON) {
				if ((os_time_get() - t) >= LED_DUTY_HI_PERIOD) {
					gpio_hal_set_red_led(DISABLE);
					led_light[LED_RED].state = LED_OFF;
					t = os_time_get();
				}
			} else {
				if ((os_time_get() - t) >= LED_DUTY_LOW_PERIOD) {
					gpio_hal_set_red_led(ENABLE);
					led_light[LED_RED].state = LED_ON;
					t = os_time_get();
				}
			}
		}
	}
}


void task_led_ctrl_start((void*) paras)
{
	thread_create(task_led_ctrl, (void *)&paras, TASK_LEDCTRL_PRIO, 0, TASK_LEDCTRL_STACK_SIZE, "task_ui");
}

