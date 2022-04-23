#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "ulog.h"
#include "linklist.h"
#include "conf.h"
#include "ethernet.h"
#include "ustring.h"
#include "forward.h"
#include "forward_udp.h"

static addr_node_t **forward_pool;

static int forward_pool_size=0;

static int (*send_fun)(const addr_node_t **,const int,const char *,const int);

/*
加载配置文件
*/
int forward_conf_load()
{
	const char *val=NULL;
	if((val=conf_get(FORWARD_URL))!=NULL && strlen(val)>0)
	{
		char forward_url[256];
		snprintf(forward_url,strlen(val)+1,"%s",val);
//debug("forward_url:%s",forward_url);
		char *token=NULL;

		token=strtok(forward_url,FORWARD_SPLIT);
		while(token!=NULL){
//			debug("token:%s",token);
			forward_pool_size++;
			token=strtok(NULL,FORWARD_SPLIT);
		}
//debug("forward_pool_size:%d",forward_pool_size);
		forward_pool = (addr_node_t **) malloc(sizeof(addr_node_t *)*forward_pool_size);
		//debug("forward_pool %p",forward_pool);
		int k=0;
		char buf[15];
		memset(buf,'\0',sizeof(buf));

		snprintf(forward_url,strlen(val)+1,"%s",val);
//		debug("forward_url:%s",forward_url);

		token=strtok(forward_url,FORWARD_SPLIT);
		while(token!=NULL){
//			debug("token:%s",token);
			int i=indexOf(token,":");

			addr_node_t *addr_node = (addr_node_t *) malloc(sizeof(addr_node_t));

			substring(buf,token,0,i);
			snprintf(addr_node->ip_addr,sizeof(addr_node->ip_addr),"%s",buf);
			memset(buf,'\0',sizeof(buf));

			substring(buf,token,i+1,strlen(token));
			addr_node->port=atoi(buf);
			//debug("addr_node->ip_addr:%s addr_node->port:%d %p %p %p",buf);

			*(forward_pool+k) = addr_node;

			//debug("addr_node->ip_addr:%s addr_node->port:%d %p %p %p",addr_node->ip_addr,addr_node->port,addr_node,*(forward_pool+k),forward_pool+k);

			k++;
			token=strtok(NULL,FORWARD_SPLIT);
		}
	}
	return 0;
}

void node_init(addr_node_t *addr_node)
{
	//创建socket对象
	addr_node->sockfd=socket(AF_INET,SOCK_DGRAM,0);
	addr_node->addr.sin_family=AF_INET;
	addr_node->addr.sin_port=htons(addr_node->port);
	addr_node->addr.sin_addr.s_addr=inet_addr(addr_node->ip_addr);
}

void node_bind()
{
	for(int i=0;i<forward_pool_size;i++)
	{
		addr_node_t *addr_node = *(forward_pool+i);
		//debug("addr_node->ip_addr:%s addr_node->port:%d %p %d",addr_node->ip_addr,addr_node->port,addr_node,i);
		node_init(addr_node);
	}
}

//默认转发方法
int forward_fun(const addr_node_t **forward_pool,const int forward_pool_size,const char *msg,const int len)
{
	debug("forward_default log:%s",msg);
	return 0;
}

/*
转发器初始化
*/
int forward_init()
{
	send_fun = forward_fun;

	forward_conf_load();
//debug("forward_pool_size:%d",forward_pool_size);
	node_bind();

	send_fun = forward_udp_send;

	return 0;
}

/*
设置转发器
*/
int set_forward_pool(const addr_node_t **set_forward_pool,const int set_forward_pool_size)
{
	forward_pool=(addr_node_t **)set_forward_pool;
	forward_pool_size=set_forward_pool_size;

	node_bind();
	return 0;
}

int forward_send(const char *msg,const int len)
{
	return send_fun((const addr_node_t **)forward_pool,forward_pool_size,msg,len);
}

/*
转发器回收
*/
int forward_release()
{
	for(int i=0;i<forward_pool_size;i++)
	{
		addr_node_t *addr_node = *(forward_pool+i);
		close(addr_node->sockfd);
		free(addr_node);
	}

	free(forward_pool);
	forward_pool_size=0;
	return 0;
}

/*
发送以太帧数据
*/
int forward_send_ethernet(db_ip_t *db_ip)
{
	char buf[512];
	int k=0;

	ustring *u_str=ustring_new();

	snprintf(buf,sizeof(buf),"%d###",db_ip->branch_id);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->fpcap_name);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->fpcap_id);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->frame_tv);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->frame_len);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->eth_src);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->eth_dst);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"%s###",db_ip->eth_length);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_vhl);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_tos);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_src);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_dst);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_length);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_protocol);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_src_port);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->ip_dst_port);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	snprintf(buf,sizeof(buf),"%s###",db_ip->alert_msg);
	ustring_append(u_str,buf,strlen(buf));
	k += strlen(buf);

	ustring_append(u_str,db_ip->payload,db_ip->payload_len);
	k += db_ip->payload_len;
	ustring_append(u_str,"###",3);
	k += 3;

	//正常版本
	char *ptr = ustring_rebuild(u_str);
	//debug("\n%s",ptr);
	forward_send(ptr,k);
	ustring_free(u_str);
/*
	//实验室PLC特殊版本
	char tsname[20];
	t_stime(tsname);
	forward_send(tsname,20);
*/
	return 0;
}
/*
int forward_send_ethernet(db_ip_t *db_ip)
{
	char buf[512];

	ustring *u_str=ustring_new();

	ustring_append(u_str,"{",1);
	snprintf(buf,sizeof(buf),"\"branch_id\":\"%d\",",db_ip->branch_id);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"fpcap_name\":\"%s\",",db_ip->fpcap_name);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"fpcap_id\":\"%s\",",db_ip->fpcap_id);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"frame_tv\":\"%s\",",db_ip->frame_tv);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"frame_len\":\"%s\",",db_ip->frame_len);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"eth_src\":\"%s\",",db_ip->eth_src);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"eth_dst\":\"%s\",",db_ip->eth_dst);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"eth_length\":\"%s\",",db_ip->eth_length);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_vhl\":\"%s\",",db_ip->ip_vhl);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_tos\":\"%s\",",db_ip->ip_tos);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_src\":\"%s\",",db_ip->ip_src);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_dst\":\"%s\",",db_ip->ip_dst);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_length\":\"%s\",",db_ip->ip_length);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_protocol\":\"%s\",",db_ip->ip_protocol);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_src_port\":\"%s\",",db_ip->ip_src_port);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"ip_dst_port\":\"%s\",",db_ip->ip_dst_port);
	ustring_append(u_str,buf,strlen(buf));

	snprintf(buf,sizeof(buf),"\"payload\":\"");
	ustring_append(u_str,buf,strlen(buf));

	ustring_append(u_str,db_ip->payload,db_ip->payload_len);
	ustring_append(u_str,"\"",1);
	ustring_append(u_str,"}",1);

	char *ptr = ustring_rebuild(u_str);
	//debug("\n%s",ptr);
	forward_send(ptr,strlen(ptr));

	ustring_free(u_str);

	return 0;
}
*/
