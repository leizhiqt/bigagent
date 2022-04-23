#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "ulog.h"
#include "times.h"
#include "thpool.h"
#include "protocol.h"

#define SERVPORT 10086 /*服务器监听端口号 */
#define BACKLOG 10 /* 最大同时连接请求数 */

static int stop = 0;
static pthread_mutex_t s_mutex;

static	thpool_t thpool;
/*
void sigroutine(int sigmsg)
{	//信号处理例程 sigmsg得到信号的值
	switch (sigmsg) {
		case 1:
			printf("Get a signal -- SIGHUP ");
			break;
		case 2:
			printf("Get a signal -- SIGINT ");
			break;
		case 3:
			printf("Get a signal -- SIGQUIT ");
			break;
		default:
			printf("Get a signal:%d",sigmsg);
			break;
	}
	
	fprintf(stderr,"execute command failed: %s",strerror(errno));

	return;
}*/

void send_full(const int csockfd){
	//send_eof(csockfd);
	//关闭socket
	close(csockfd);
}

void help()
{
	fprintf(stderr, ">help/?	exit\n");
}

void* console(void *v)
{
	//task_match_t *task_match = (task_match_t *)v;

	//debug("task_match->=%p _task_match=%p",task_match,&_task_match);

	printf("====Welcome to MdServer v1.0=====\n");

	char buf[256];
	while(strcmp(buf,"exit\n")!=0)
	{
		if(strcmp(buf,"?\n")==0 || strcmp(buf,"help\n")==0)
			help();

		printf(">");
		fgets(buf,sizeof(buf),stdin);
	}

	pthread_mutex_lock(&s_mutex);
	stop = 1;
	pthread_mutex_unlock(&s_mutex);

	//task_match->stop =0;

	usleep(3000);//等待其他任务3ms

	pthread_mutex_destroy(&s_mutex);

	debug("console exit!");
	//exit(0);
	//pthread_exit(NULL);
}

void* thservice(void *v)
{
	thclient_t thclient = *(thclient_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	
	debug("thservice 开始:csockfd:%d pid:%u pthread_id:%u (0x%x)",thclient.csockfd,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);

	//执行代码
	//命令处理
	recv_shake_hand(&thclient);

	//关闭socket
	close(thclient.csockfd);

	//shutdown(mfr.csockfd, SHUT_WR);
	debug("thservice 结束:csockfd:%d pid:%u pthread_id:%u (0x%x)",thclient.csockfd,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
}

void* mhservice(void *v)
{
/*
	task_match_t *task_match = (task_match_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();

	debug("mhservice 开始 pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);

	debug("task_match->=%p",task_match);
	matchVideo(task_match);

	debug("mhservice 结束 pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	*/
}

void start(void *v)
{
	//task_match_t *task_match = (task_match_t *)v;

	int ssockfd;
	struct sockaddr_in my_addr; /* 本机地址信息 */
	struct sockaddr_in remote_addr; /* 客户端地址信息 */
	struct timeval tv;

	static int timeout = 60;

	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	if ((ssockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket error");
		exit(1);
	}

	//close now
	struct linger so_linger;
	so_linger.l_onoff = 1;
	so_linger.l_linger = 0;
	setsockopt(ssockfd,SOL_SOCKET,SO_LINGER,&so_linger,sizeof so_linger);

	//设置发送超时
	setsockopt(ssockfd,SOL_SOCKET,SO_SNDTIMEO,&tv,sizeof(struct timeval));
	//设置接收超时
	setsockopt(ssockfd,SOL_SOCKET,SO_RCVTIMEO,&tv,sizeof(struct timeval));

	//非阻塞模式
	//int flags = fcntl(ssockfd, F_GETFL, 0);
	//fcntl(ssockfd,F_GETFL,flags|O_NONBLOCK );

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=htons(SERVPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(my_addr.sin_zero),8);
	if (bind(ssockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind error");
		exit(1);
	}
	if (listen(ssockfd, BACKLOG) == -1) {
		perror("listen error");
		exit(1);
	}

	unsigned int sin_size = sizeof(struct sockaddr_in);
	int c = 0;
	
	fd_set fset;
	while(1)
	{
		pthread_mutex_lock(&s_mutex);
		if(stop) break;//exit
		pthread_mutex_unlock(&s_mutex);

		tv.tv_sec = timeout;
		tv.tv_usec = 0;

		FD_ZERO(&fset);
		FD_SET(ssockfd,&fset);
		c=select(ssockfd+1,&fset,NULL,NULL,&tv);

		if(c==0)
		{
			usleep(1000);//每秒1000次处理
			continue;
		}
		else if(c<0)
		{
			break;
		}

		if(FD_ISSET(ssockfd,&fset)) // new connection
		{
			//传值到客户端
			thclient_t thclient;
			//thclient.task_match = task_match;

			if ((thclient.csockfd = accept(ssockfd, (struct sockaddr *)&remote_addr,&sin_size)) == -1)
			{
				perror("accept error");
				if(errno == EAGAIN)
					continue;
			}

			debug("received a connection from %s",inet_ntoa(remote_addr.sin_addr));

			//thservice 加入任务队列
			if(thpool_add_work(&thpool,(void*)thservice, (void*)&thclient)<0)
			{
				debug("thpool full",__FILE__,__LINE__);
				send_full(thclient.csockfd);
			}
		}
		//sleep(1);
	}

	close(ssockfd);
	debug("start exit");
}

void usage()
{
	//fprintf(stderr, "server\t?\n");
	//fprintf(stderr, "\t--help\n");
	fprintf(stderr, "\t-l\n");
}

int main(int argc,char *argv[])
{
	//setenv("MALLOC_TRACE", "server-trace.log",1);
	//mtrace();  

	logInit("exchange.log",0);

	//conf_def();

	//debug(" argc=%d",argc);
	if( argc < 2)
	{
		usage();
		return -1;
	}

	//环境变量
	/*for( int i = 0; envp[i] != NULL; ++i )
	{
		printf( "%d : %s \n", i, envp[i] );
	}*/

	int k = 0;
	//printf("server");
	while (argc > k) {
		//printf("%s\n",argv[k]);
		if ((strcmp(argv[k],"?") == 0) || (strcmp(argv[k],"--help") == 0)){
			k++;
			usage();
			return 0;
			//c = atoi(argv[k]);
		}

		/*if (strcmp(argv[k],"-ntp") == 0){
			k++;
			snprintf(ntp_ip,sizeof(ntp_ip),"%s",argv[k]);
			printf(" -ntp %s",ntp_ip);
			//c = atoi(argv[k]);
		}*/

		/*if (strcmp(argv[k],"-d") == 0){
			k++;
			snprintf(rpath,sizeof(rpath),"%s",argv[k]);
			printf(" -d %s",rpath);
		}*/

		/*if (strcmp(argv[k],"-f") == 0){
			k++;
			fk = atoi(argv[k]);
			printf(" -f %d",fk);
		}*/

		/*if (strcmp(argv[k],"-v") == 0){
			k++;
			snprintf(vfpath,sizeof(vfpath),"%s",argv[k]);
			printf(" -v %s",vfpath);
		}*/

		if (strcmp(argv[k],"-log") == 0){
			k++;
			logInit(argv[k],0);
		}

		k++;
	}
	printf("\n");

	//conf_init();

	/*//下面设置三个信号的处理方法
	signal(SIGHUP, sigroutine);
	signal(SIGINT, sigroutine);
	signal(SIGQUIT, sigroutine);
*/
	//任务数据
	//task_init(10);

	//线程池
	thpool_init(&thpool,25,5);
	usleep(50);

	//异常监控 mhservice
	//thpool_add_work(&thpool,(void*)mhservice, (void*)task_match);

	//console 加入任务队列
	//thpool_add_work(&thpool,(void*)console, (void*)task_match);
	thpool_add_work(&thpool,(void*)console, NULL);

	//开启Socket服务
	start(NULL);

	debug("thpool_destroy");

	thpool_destroy(&thpool);

	//等待其他线程执行完
	sleep(1);

	debug("Main App exit");

	return 0;
}
