#include "common.h"
#include "lwip/netif.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"


static char* hostname = "test.ring.com";
#define SERVER_PORT			13469
#define UDP_CLIENT_PERIOD	1000 //ms
#define RECV_TIMEOUT		500	//ms


//________static api ________
static void udp_socket_set(int en);
static int udp_socket_state(void);
//___________________________
typedef enum{
	evt_null,
	evt_socket_open,
	evt_socket_close,	
}udp_client_event;

static BOOL udp_client_socket_en = DISABLE;
static udp_client_event udp_client_evt = evt_null;	
mbox_t mbox_udp_client_evt = NULL;

static const udp_client_cmd_object udp_cmd_obj = {
	udp_socket_set,
	udp_socket_state,
};

const udp_client_cmd_object* p_udp_cmd_ctrl = &udp_cmd_obj;

static void udp_socket_set(int en)
{
	if (mbox_udp_client_evt != NULL) {
		if(en <= 0) {
			udp_client_evt = evt_socket_close;
			udp_client_socket_en = DISABLE;
		} else {
			udp_client_evt = evt_socket_open;
			udp_client_socket_en = ENABLE;
		}
		mbox_post(&mbox_udp_client_evt, (void *)&udp_client_evt);
	}
}

static int udp_socket_state(void)
{
	return udp_client_socket_en;
}


int udp_client_init(void)
{
	int socket_c, err;
	struct sockaddr_in addr;
	socket_c = socket(AF_INET, SOCK_DGRAM, 0);
	if (socket_c < 0) {
		printf("get socket err:%d", errno);
		return  -1;
	}

	//bind the socket to any valid IP address and a speicify port
	struct sockaddr_in myaddr;
	memset((char *)&myaddr, 0, sizeof(myaddr));
	myaddr.sin_family = AF_INET;
	myaddr.sin_len = sizeof(struct sockaddr_in);	
	myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	myaddr.sin_port = htons(SERVER_PORT);	//assign this port number as same as RING server.

    if (bind(socket_c, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
            printf("bind failed");
            return -1;
    }

	printf("link ok\n");
	return socket_c;
}


static void task_udp_client(void* pdata)
{
	unsigned char err;
	const system_paras *p_paras = (sys_para  *)pdata;
	mbox_new(&mbox_udp_client_evt, (void *)0);
	udp_client_event* p_evt;
	
	struct sockaddr_in server_addr;
	int len;
	int socket_fd;
	int ret_select;
	fd_set master, read_fd;
	struct timeval sel_timeout;
	char recv_buf[2], send_buf[2];

	short *ptr_sendbuf = (short*)send_buf;
	short *ptr_recvbuf = (short*)recv_buf;
	socklen_t addrlen = sizeof(server_addr);
	memset((char *)&server_addr, 0, sizeof(server_addr));

	struct hostent* h;
	// get server IP address by DNS name or input a address
	h = gethostbyname(hostname);
	if(h !=NULL) {
		printf("%s: sending data to '%s' (IP : %s) \n", hostname, h->h_name,inet_ntoa(*(struct in_addr *)h->h_addr_list[0]));
		server_addr.sin_family = h->h_addrtype;
		memcpy((char *) &server_addr.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
	} else {
		printf("unknown host '%s' \n", hostname);
		//specify a IP address
		server_addr.sin_family = AF_INET;
		server_addr.sin_addr.s_addr = inet_addr("xxx.xxx.xxx.xxx");	//Angus , assign a IP address
	}

	server_addr.sin_len = sizeof(struct sockaddr_in);
	server_addr.sin_port = htons(13469);
	sel_timeout.tv_sec = 0;
	sel_timeout.tv_usec= RECV_TIMEOUT*1000;
	FD_ZERO(&read_fd);
	FD_ZERO(&master);
	while(1) {
		p_evt = mbox_get(&mbox_udp_client_evt, 0);
		if (p_evt != NULL && *p_evt == evt_socket_open && p_paras->battery_low == TRUE) {
			socket_fd = udp_client_init();
			if (socket_fd == -1) {
				printf("udp_client_init err\n");
			} else {
				printf("udp_client_init ok\n");
				FD_SET((int)socket_fd, &master);
				//clear send/recv buf
				*ptr_sendbuf = 0;
				*ptr_recvbuf = 0;
				while(1) {
					p_evt = mbox_get(&mbox_udp_client_evt, UDP_CLIENT_PERIOD);
					if(p_evt != NULL && *p_evt == evt_socket_close)
						break;
					
					sendto(socket_fd,send_buf,sizeof(send_buf),0,(struct sockaddr*)&server_addr,sizeof(server_addr));
					(*ptr_sendbuf)++;		
					read_fd = master;
					ret_select = select(socket_fd+1 , &read_fd, NULL, NULL,&sel_timeout);
					if (ret_select > 0) {
						printf("ret_select!!\n");
						if (FD_ISSET((int)socket_fd, &read_fd)) {
							printf("recv : \n");
							len = recvfrom(socket_fd, recv_buf, sizeof(recv_buf), 0, (struct sockaddr *)&server_addr, &addrlen);
							if(len > 0) {
								if (*ptr_recvbuf != *ptr_sendbuf) {
									//error val, It should send the last count value again
									*ptr_sendbuf = (*ptr_sendbuf > 0) ? --(*ptr_sendbuf) : 0;
								}
							} else {
								break;
							}
						}
					} else {
						//recv timeout, It should send the last count value again
						*ptr_sendbuf = (*ptr_sendbuf > 0) ? --(*ptr_sendbuf) : 0;
					}
					
				}
				FD_CLR((int)socket_fd, &master);
				close(socket_fd);
			}
		}
	}
}


void task_udpclient_start((void*) paras)
{
	thread_create(task_key, paras, TASK_UDPCLIENT_PRIO, 0, TASK_UDPCLIENT_STACK_SIZE, "task_udp_client");
}

