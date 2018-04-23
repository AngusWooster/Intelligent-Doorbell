#ifndef _UDPCLIENT_H_
#define _UDPCLIENT_H_


typedef struct{
	void (*udp_socket_set)(int en);
	int (*udp_socket_state)(void);
}udp_client_cmd_object;

void task_udpclient_start((void*) paras);

#endif

