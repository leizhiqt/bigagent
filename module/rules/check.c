#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <epan/epan_dissect.h>
#include <epan/addr_resolv.h>
#include <epan/charsets.h>

#include "ulog.h"
#include "linklist.h"
#include "conf.h"

#include "thpool.h"
#include "ethernet.h"
#include "db_ethernet.h"
#include "db_service.h"
#include "ulibpcap/cf_pcap.h"
#include "extra/forward.h"
#include "extra/slave.h"
#include "check.h"

/*default white
	*0		white
	*1		black
*/
//默认白名单
static int rule_list=1;

//规则配置文件
static char *conf_path="/tmp/agent_rules.txt";

//通用配置对象
static link_list_t *u_conf=NULL;

//开辟线程池
static	thpool_t thpool;

/*
规则初始化
*/
void rules_init()
{
	u_conf=uconf_new();
	uconf_load(u_conf,conf_path);

	//线程池
	thpool_init(&thpool,75,5);
	usleep(50);
}

/*
规则释放
*/
void rules_destroy()
{
	uconf_destroy(u_conf);

	//释放线程池		阻塞等待任务执行完
	thpool_destroy(&thpool);
}

/*字符串检查
	字符串ptr中是否包含 flags经过flags_split分割后的子串
	return	0	不匹配
			1	匹配
*/
int check_str_split(const char *ptr,const uint len,const char *flags,const uint flags_len,const char *flags_split)
{
	//debug("flags %s",flags);
	if(flags==NULL || flags_split==NULL || len<=0 || flags_len<=0 || strlen(flags) <=0)
	{
		//debug("check_str_split 0");
		return 0;
	}

	char flags_buf[flags_len+1];
	snprintf(flags_buf,flags_len+1,"%s",flags);

	//debug("flags_buf:%s len:%ld ",flags_buf,flags_len);

	char *match_str=NULL;
	char *token=strtok(flags_buf,flags_split);
	while(token!=NULL){
		//debug("token:%s",token);

		match_str=strstr(ptr,token);
		//debug("match_str:%s",match_str);

		if(match_str!=NULL){
			//db_ip->payload=match_str;
			//db_ip->payload_len=strlen(match_str);
			return 1;
		}

		token=strtok(NULL,flags_split);

		//debug("token:%s",token);
	}
	//debug("not match");
	return 0;
}

//协议检查	0_1		不匹配_匹配
int check_proto(db_ip_t *db_ip,const char *flags,const char *flags_split)
{
	int check_result=check_str_split((char *)db_ip->ip_protocol,strlen((char *)db_ip->ip_protocol),flags,strlen(flags),flags_split);
	return check_result;
}

//端口检查	0_1		不匹配_匹配
int check_port(db_ip_t *db_ip,const char *flags,const char *flags_split)
{
	int check_result=check_str_split((char *)db_ip->ip_dst_port,strlen((char *)db_ip->ip_dst_port),flags,strlen(flags),flags_split);
	return check_result;
}

//payload检查	0_1		不匹配_匹配
int check_payload(db_ip_t *db_ip,const char *ptr,const int len,const char *flags,const char *flags_split)
{
	int check_result=check_str_split(ptr,len,flags,strlen(flags),flags_split);
	return check_result;
}

//端口检查	0_1		不匹配_匹配
int check_host(db_ip_t *db_ip,const char *flags,const char *flags_split)
{
	int check_result=check_str_split((char *)db_ip->ip_dst,strlen((char *)db_ip->ip_dst),flags,strlen(flags),flags_split);
	return check_result;
}

//black_white 0 1 2
int set_rule_list(int black_white)
{
	return 0;
}

//db_ethernet_add & forward_send_ethernet
void check_do(db_ip_t *db_ip)
{
	//写入数据库
	db_ethernet_add(db_ip);
	//debug("db_ethernet_add");
	//转发告警
	forward_send_ethernet(db_ip);
	//debug("forward_send_ethernet");
}

//上传线程
void *up_pthread(void *v)
{
	const db_ip_t db_ip= * (const db_ip_t *) v;

	const char *up_ip = conf_get(CONF_SLAVE_KEY);

	int n_secs=1;
	const char *u_val=uconf_get(u_conf,RULE_U);

	if(u_val!=NULL)
		n_secs = atoi(u_val);

	char cf_name[20];
	snprintf(cf_name,sizeof(cf_name),"%lu.pcap",db_ip.t_secs);

	cf_pcap_split_tm(cf_name,db_ip.t_secs,n_secs);

	//debug("up_to %s %d",up_ip,SLAVE_PORT);

	fclient(up_ip,SLAVE_PORT,cf_name,strlen(cf_name));

	return NULL;
}

//最后所有入库
//db_ip_t *db_ip
int check_up_pcap(const db_ip_t *db_ip)
{
//const char *fpcap_name,const time_t t_secs

	static time_t previous_t=0;

	if(previous_t==db_ip->t_secs)
	{
		return 0;
	}

	previous_t=db_ip->t_secs;

	if(thpool_add_work(&thpool,up_pthread,(void *)db_ip)<0)
	{
		debug("thpool full");
	}

	return 0;
}

//最后所有入库
int check_db_add(db_ip_t *db_ip, const char *ptr,const int len)
{
	//入库
	db_ip->payload=ptr;
	db_ip->payload_len=len;
	check_do(db_ip);
	return 0;
}

/*字符串检查
	return	0	默认值全量存储
			>0	按匹配规则存储
*/
int to_check_flags(db_ip_t *db_ip, const char *ptr,const int len,const char *flags_split)
{
	//传入参数检查
	if(flags_split==NULL || db_ip==NULL || db_ip->ip_protocol==NULL)
	{
		debug("to_check_flags in parameter not legal");
		return 0;
	}

	const char *rule_type=uconf_get(u_conf,RULE_LIST);
	if(rule_type!=NULL)
	{
		rule_list=atoi(rule_type);
	}

	//const char *proto_flags="udp;tcp";
	const char *proto_flags=uconf_get(u_conf,RULE_PROTO);
	//debug("proto_flags %s",proto_flags);

	//port 12101 and  proto udp
	//const char *port_flags="12101;80";
	const char *port_flags=uconf_get(u_conf,RULE_PORT);
	//debug("port_flags %s",port_flags);

	//const char *payload_flags="File Transfer Protocol (FTP);FTP Data";
	const char *payload_flags=uconf_get(u_conf,RULE_PAYLOAD);
	//debug("port_flags %s",payload_flags);

	debug("rule_list %d",rule_list);
	//白名单
	if(rule_list==1)
	{
		//全部是异常	白名单没有过滤条件
		if(proto_flags==NULL && port_flags==NULL && payload_flags==NULL)
		{
			//snprintf(db_ip->alert_msg,sizeof(db_ip->alert_msg),"match_normal");
			check_db_add(db_ip,ptr,len);
			return 0;
		}

		//检查结果合法		白名单判断过滤条件
		if(check_proto(db_ip,proto_flags,flags_split)
			|| check_port(db_ip,port_flags,flags_split)
			|| check_payload(db_ip,ptr,len,payload_flags,flags_split)
			)
		{
			return 0;
		}

		debug("check_white");
		snprintf(db_ip->alert_msg,sizeof(db_ip->alert_msg),"match_white");
		check_db_add(db_ip,ptr,len);
		check_up_pcap(db_ip);
		return 3;
	}

	//黑名单
	if(rule_list==0)
	{
		//全部合法 无须入库
		if(proto_flags==NULL && port_flags==NULL && payload_flags==NULL)
		{
			return 0;
		}

		debug("check_proto");
		if(check_proto(db_ip,proto_flags,flags_split))
		{
			snprintf(db_ip->alert_msg,sizeof(db_ip->alert_msg),"match_proto");
			check_db_add(db_ip,ptr,len);
			check_up_pcap(db_ip);
			return 1;
		}

		debug("check_port");
		if(check_port(db_ip,port_flags,flags_split))
		{
			snprintf(db_ip->alert_msg,sizeof(db_ip->alert_msg),"match_port");
			check_db_add(db_ip,ptr,len);
			check_up_pcap(db_ip);
			return 2;
		}

		debug("check_payload");
		if(check_payload(db_ip,ptr,len,payload_flags,flags_split))
		{
			snprintf(db_ip->alert_msg,sizeof(db_ip->alert_msg),"match_payload");
			check_db_add(db_ip,ptr,len);
			check_up_pcap(db_ip);
			return 3;
		}
	}

	//debug("ptr:%s len:%d ",ptr,len);
	debug("check_default_all Not insert DB");

	return 0;
}

/*规则检查
	return	0	默认值	全协议解析	存储
			>0	匹配规则	后的字符串	存储
*/
int rules_check(db_ip_t *db_ip, const char *ptr,const int len)
{
	return to_check_flags(db_ip,ptr,len,RULE_SPLIT_SEMICOLON);
	//return to_check_flags(db_ip,ptr,len,RULE_SPLIT_OR);
}
