#ifndef EXTRA_FORWARD_H
#define EXTRA_FORWARD_H

#define FORWARD_URL	"forward_url"
#define FORWARD_SPLIT	";"

typedef struct _addr_node{
	char ip_addr[15];
	int port;

	struct sockaddr_in addr;
	int sockfd;
}addr_node_t;

/*
加载配置文件
*/
int forward_conf_load();

/*
设置转发器
*/
int set_forward_pool(const addr_node_t **set_udp_pool,const int set_pool_size);

/*
转发器初始化
*/
int forward_init();

/*
给转发器发送消息
*/
int forward_send(const char *msg,const int len);

/*
发送以太帧数据
*/
int forward_send_ethernet(db_ip_t *db_ip);

/*
转发器回收
*/
int forward_release();

#endif
