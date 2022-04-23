#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <pcap.h>

#include "ulog.h"
#include "dtype.h"

#include "linklist.h"
#include "thpool.h"
//#include "times.h"
#include "ufile.h"

//mongodb
//#include "mongodbconn.h"
#include <mysql.h>

#include "mysqlconn.h"
#include "ethernet.h"
#include "db_ethernet.h"
#include "db_service.h"
#include "extra/forward.h"
#include "ulibpcap/dumppcap.h"

#include "uboot.h"

//开辟线程池
boot_parameters_t para;

//boot_start
int (*fun_init) ();

int (*fun_destroy) ();

//处理方法
int (*fun_read_pcap) (const char *,u_short);

// 会话句柄
static	pcap_t *handle;

//pcap文件队列
static link_list_t link_list;

//开辟线程池
static	thpool_t thpool;

void boot_process_stop()
{
	debug("stop");
	para.stop = 1;
	pcap_breakloop(handle);
}

void *pcap_breakloop_pthread(void *v)
{
	while(1)
	{
		pcap_breakloop(handle);
		sleep(30);
	}
}

//para_init
void para_init()
{
	debug("para_init");

	logInit("uboot.log",1);
	para.branch_id = 100;
	para.status = 'N';
	memset(para.dev,'\0',sizeof(para.dev));
	snprintf(para.save_pcap,sizeof(para.save_pcap),"%s","/tmp");
	memset(para.filter_rule,'\0',sizeof(para.filter_rule));
	snprintf(para.lock_file,sizeof(para.lock_file),"%s","/tmp/agent.lock");
	para.stop = 0;
}

//解析pcap文件存放的目录
void ufile_list(char *path,u_short branch_id)
{
	debug("ufile_list");
	DIR		*pDir; 
	struct dirent	*ent; 
	//int		i=0; 
	char		childpath[512]; 
	struct stat s_buf;
	/*获取文件信息，把信息放到s_buf中*/
	stat(path,&s_buf);

	//是普通文件
	if(S_ISREG(s_buf.st_mode))
	{
		if(strstr(path,".pcap")!=NULL)
		{
			//printf("path:%s\n",childpath);
			debug("Process %s",path);
			if(fun_read_pcap!=NULL)
				fun_read_pcap(path,branch_id);
		}
	}

	//处理目录
	if(!S_ISDIR(s_buf.st_mode)) return;

	pDir=opendir(path); 
	memset(childpath,0,sizeof(childpath));

	while((ent=readdir(pDir))!=NULL)
	{ 
		if(ent->d_type & DT_DIR)
		{ 
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
				continue; 
			//debug(childpath,"%s/%s",path,ent->d_name);
			//debug("path:%s\n",childpath);
			//ufile_list(childpath,branch_id,fun);
		} 
		else
		{
			//debug("path:%s\n",ent->d_name);
			if(strstr(ent->d_name,".pcap")!=NULL)
			{
				memset(childpath,0,sizeof(childpath));
				sprintf(childpath,"%s/%s",path,ent->d_name);
				//debug("Process %s",childpath);
				if(fun_read_pcap!=NULL)
					fun_read_pcap(childpath,branch_id);
			}
		}
	}
	closedir(pDir);
}

//资源初始化
void initialize(int (*_fun_init) (),int (*_fun_destroy) (),int (*_fun_read_pcap) (const char *,u_short))
{
	//int k=-1;

	if(_fun_init!=NULL)
	{
		fun_init = _fun_init;
		fun_init();
	}
debug("initialize");
	if(_fun_destroy!=NULL)
	{
		fun_destroy = _fun_destroy;
	}

	//初始化处理方法
	if(_fun_read_pcap!=NULL)
	{
		fun_read_pcap = _fun_read_pcap;
	}

	//线程池
	thpool_init(&thpool,75,5);
	usleep(50);

	//初始化队列
	link_list_init(&link_list);

	mysql_conn_init(mysql_conn_get_default());

	#ifdef DB_MONGO_H
	mongodb_conn_init();
	#endif

	//初始化表
	db_ethernet_init();
	
	//告警外发
	forward_init();
}

//
//遍历链表
int queue_forth(link_list_t *link_list)
{
	link_node_t *p;
	pcap_node_t *pcap_node_p;

	int n=0;
	while(1){
		p = link_list->head;
		n = link_list_size(link_list);

		if(p==NULL || n==0)
		{
			sleep(15);
			continue;
		}

		debug("queue_remove_first %p size:%d",p,n);

		pcap_node_p = container_of(p,pcap_node_t,node);
		link_list_remove_first(link_list,pcap_node_t);

		if(fun_read_pcap!=NULL)
			fun_read_pcap(pcap_node_p->buf,para.branch_id);

		debug("Handle fun_read_pcap %s complete para.stop %d",pcap_node_p->buf,para.stop);

		free(pcap_node_p);

		if(para.stop) break;

		//等待抓取
		sleep(3);
	}
	debug("queue_forth complete");
	return 0;
}

//readpcap_ethernet
void *read_pcap_pthread(void *v)
{
	//int k=-1;

	//client_t* client = (client_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	debug("read_pcap_pthread 开始:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);

	queue_forth(&link_list);

	sleep(1);
	debug("read_pcap_pthread 结束:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	
	return NULL;
}

//write_pcap_pthread
void *write_pcap_pthread(void *v)
{
	//int k=-1;

	//client_t* client = (client_t *)v;

	char *fpath = (char *)v;
	
	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	debug("write_pcap_pthread 开始:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);

	if(fpath==NULL) exit(0);

	debug("promiscuous save_path:%s",fpath);
	//抓包
	promiscuous(fpath,NULL,NULL);

	sleep(1);
	debug("write_pcap_pthread 结束:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	//pthread_exit(NULL);
	return NULL;
}

//多线程服务启动
void service_start(char *fpath)
{
	debug("Start service");

	if(thpool_add_work(&thpool,write_pcap_pthread,fpath)<0)
	 {
		 debug("thpool full");
	 }
	sleep(3);

	if(thpool_add_work(&thpool,read_pcap_pthread,NULL)<0)
	 {
		 debug("thpool full");
	 }
	sleep(3);

	/*
	if(thpool_add_work(&thpool,pcap_breakloop_pthread,NULL)<0)
	 {
		 debug("thpool full");
	 }
	sleep(1);*/

	debug("Success service");
}


//资源释放
void release()
{
	//释放线程池		阻塞等待任务执行完
	thpool_destroy(&thpool);

	if(fun_destroy!=NULL)
	{
		fun_destroy();
	}

	//释放队列
	link_list_destroy(&link_list,pcap_node_t);

	//mysql
	mysql_conn_release(mysql_conn_get_default());

	//mongodb
	#ifdef DB_MONGO_H
	mongodb_conn_release();
	#endif

	if(access(para.lock_file,F_OK|R_OK)!=-1)
	{
		remove(para.lock_file);
	}

	forward_release();

	debug("Success release");
}
