#ifndef EXTRA_SLAVE_H
#define EXTRA_SLAVE_H

#define BUFF_LIEN_SIZE 4096
#define SLAVE_PORT 6666

#define CONF_MASTER_KEY "master_enable"
#define CONF_SLAVE_KEY  "slave_enable"

/*
slave与master通信引擎
*/

/*
tcp 客户端上		传文件
*/
int fclient(const char *ip_addr,const int port,const char *file_name,const int file_name_len);

/*
tcp 服务端接		收文件
*/
int fserver(const int port);

void *fserver_pthread(void *v);

#endif
