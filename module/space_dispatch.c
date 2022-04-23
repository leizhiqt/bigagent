#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/statfs.h>
#include <stdio.h>
#include <stdint.h>

#include "ulog.h"
#include "linklist.h"
#include "conf.h"

//#include "times.h"
#include "thpool.h"
#include "ufile.h"

#include <mysql.h>

#include "mysqlconn.h"
#include "ethernet.h"
#include "db_ethernet.h"
#include "db_service.h"

//启动参数
typedef struct _boot_parameters{
	//保存时间 天
	u_short max_day;

	//保存最近最大记录数
	u_long max_record;

	//磁盘使用率
	float disk_threshold;

	//间隔时间秒
	u_short interval_time;

	char status;
	char lock_file[128];

	const char *conf;

	//停止
	int stop;
}boot_parameters_t;

static boot_parameters_t para;

static int pid;

//开辟线程池
static	thpool_t thpool;

//para_init
void para_init()
{
	debug("para_init");

	logInit("uboot.log",1);
	para.status = 'N';

	para.max_day=7;
	para.max_record=500000;
	para.disk_threshold=0.6;
	para.interval_time=360;

	snprintf(para.lock_file,sizeof(para.lock_file),"%s","/tmp/space.lock");
	para.stop = 0;
}

void para_load()
{
	if(!conf_enable()) return;
	logInit(conf_get("log_file"),1);

	para.max_day = atoi(conf_get("max_day"));
	para.max_record = atoi(conf_get("max_record"));
	para.disk_threshold = atof(conf_get("disk_threshold"));
	para.interval_time = atoi(conf_get("interval_time"));
	snprintf(para.lock_file,sizeof(para.lock_file),"%s",conf_get("pid"));

	//debug("para_load");
}

//中断	send_signal
void para_check()
{
	debug("para_check");
	//pid文件不存在
	if(access(para.lock_file,F_OK|R_OK)==-1)
	{
		debug("%s 文件不存在",para.lock_file);
		if(para.status=='T' || para.status=='R'){
			exit(0);
		}
	}

	char lines[256];
	int k=read_line(para.lock_file,lines,sizeof(lines));
	lines[k]='\0';
	long pid = atol(lines);

	if(para.status=='T'){
		debug("stop pid %lu",pid);
		kill(pid, SIGUSR1);
		exit(0);
	}

	if(para.status=='R'){
		debug("restart pid:%lu",pid);
		kill(pid, SIGUSR1);
		sleep(1);
	}
}

//磁盘使用率
//#define KB 1024.0										// 2^10
//#define MB 1048576.0									// 2^20
//#define GB 1073741824.0								// 2^30

double
disk_usage()
{
	struct statfs diskInfo;

	statfs("/", &diskInfo);

	uint64_t blocksize = diskInfo.f_bsize;					// 每个block里包含的字节数
	uint64_t totalsize = blocksize * diskInfo.f_blocks;		// 总的字节数，f_blocks为block的数目

	/*printf("Disk_size = %ldB %fKB %fMB %fGB\n",
		totalsize,
		totalsize/KB,
		totalsize/MB,
		totalsize/GB);*/

	//uint64_t freeDisk = diskInfo.f_bfree * blocksize;		//剩余空间的大小
	uint64_t availableDisk = diskInfo.f_bavail * blocksize;	//可用空间大小

	/*printf("Disk_free = %fMB %fGB\n"
			"Disk_available = %fMB %fGB\n",
		freeDisk/MB,
		freeDisk/GB,
		availableDisk/MB,
		availableDisk/GB);*/

	double d = (double)availableDisk/totalsize;
	//printf("Disk_free=%ld\nDisk_available=%ld\nDisk_size=%ld\n",freeDisk,availableDisk,totalsize);
	//printf("= %lf\n",d);
	return d;
}

//启动应用程序
char *app;

void usage(char *p);

//停止进程
void boot_process_stop()
{
	debug("stop");
	para.stop = 1;
}

//启动初始化
void boot_initialize()
{
	//线程池
	thpool_init(&thpool,75,5);
	usleep(50);

	mysql_conn_init(mysql_conn_get_default());

	para_init();

	//注册信号量绑定
	//handler_int_1	接收中断--停止
	signal(SIGUSR1,boot_process_stop);

	debug("Initialize Success");
}

//加载配置文件
void boot_load()
{
	para_load();

	para_check();

	debug("Load Success");
}

//资源释放
void boot_release()
{
	//释放线程池		阻塞等待任务执行完
	thpool_destroy(&thpool);

	//mysql
	mysql_conn_release(mysql_conn_get_default());

	debug("release Success");
}

int boot_dispatch()
{
	debug("boot_action %c",para.status);

	if(para.status=='S'||para.status=='R'){
		return 0;
	}

	if(para.status=='N'){
		usage(app);
		return -1;
	}

	return -2;
}

int boot_try_lock()
{
	int ret=-1;

	pid_t self_pid = getpid();
	debug("getpid %d",self_pid);
	char pid_str[20];
	memset(pid_str,'\0',sizeof(pid_str));
	snprintf(pid_str,sizeof(pid_str),"%d\n",self_pid);

	//lock
	struct flock lock = {0};
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	//180
	for(int i=0;i<360;i++)
	{
		//文件检测
		if(access(para.lock_file,F_OK|R_OK)==-1)
		{
			//创建文件
			if((pid = open(para.lock_file,O_WRONLY|O_CREAT))==-1)
			{
				debug("Error open %s\n",para.lock_file);
				exit(-1);
			}

			if((ret = fcntl(pid, F_SETLK, &lock))==-1)
			{
				debug("lock %d\n",ret);
			}

			write(pid, pid_str, strlen(pid_str));
			//sleep(100);
			return 0;
		}

		if((pid = open(para.lock_file,O_RDWR))==-1)
		{
			debug("Error open %s\n",para.lock_file);
			return -1;
		}

		if((ret = fcntl(pid, F_SETLK, &lock))!=-1)
		{
			debug("lock %d\n",ret);
			write(pid, pid_str, strlen(pid_str));
			return 0;
		}
		close(pid);

		sleep(1);
	}
	exit(0);
}

void boot_unlock()
{
	//unlock
	struct flock unlock = {0};
	unlock.l_type = F_UNLCK;
	unlock.l_whence = SEEK_SET;
	unlock.l_start = 0;
	unlock.l_len = 0;

	int ret=-1;

	if((ret = fcntl(pid, F_SETLK, &unlock))==-1)
	{
		debug("unlock %d\n",ret);
	}
	debug("unlock %d\n",ret);

	close(pid);

	if(access(para.lock_file,F_OK|R_OK)!=-1)
	{
		remove(para.lock_file);
	}
}

//
void *space_pthread(void *v)
{
	//int k=-1;

	//client_t* client = (client_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	debug("write_pcap_pthread 开始:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);

	if(para.interval_time<30) para.interval_time=30;
	if(para.disk_threshold>0.9) para.disk_threshold=0.9;
	if(para.max_record<30000) para.max_record=30000;

	//run
	while(1){
		//debug("para.stop %d %s",para.stop,para.conf);

		//刷新参数
		if(conf_load(para.conf)==0)		para_load();

		if(para.disk_threshold<disk_usage())
		{
			//清理数据库
			db_ethernet_clear(para.max_record);
		}

//SELECT STR_TO_DATE('May 21, 2019 17:35:03.717584000 CST','%b %d,%Y %H') tv from dual;
//SELECT STR_TO_DATE('May 21, 2019 17:35:03.717584000 CST','%b %d,%Y') tv from dual;

//select STR_TO_DATE(frame_tv,'%b %d,%Y') tv from  ethernet group by tv;
		if(para.stop) break;

		//间隔时间
		sleep(para.interval_time);
	}

	sleep(1);
	debug("write_pcap_pthread 结束:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	return NULL;
}

//多线程服务启动
void boot_run()
{
	debug("Start service");
/*
	if(thpool_add_work(&thpool,space_pthread,NULL)<0)
	 {
		 debug("thpool full");
	 }
	sleep(3);
*/
	space_pthread(NULL);

	debug("Success service");
}

void usage(char *p)
{
	fprintf(stdout, "%s -s/-r -c space.cnf\n",p);
	fprintf(stdout, "%s -t\n",p);
}

int main(int argc,char **argv,char **envp)
{
	if (argc < 2)
	{
		usage(*argv);
		exit(1);
	}

	app = *argv;

	//初始化
	boot_initialize();

	int k = 0;
	while (argc > k) {
		//printf("%s\n",argv[k]);
		if ((strcmp(argv[k],"?") == 0) || (strcmp(argv[k],"--help") == 0)){
			usage(*(++argv));
			return 1;
			//c = atoi(argv[k]);
		}

		if (strcmp(argv[k],"-t") == 0){
			para.status='T';
		}

		if (strcmp(argv[k],"-s") == 0){
			para.status='S';
		}

		if (strcmp(argv[k],"-r") == 0){
			para.status='R';
		}

		if (strcmp(argv[k],"-c") == 0){
			para.conf = *(argv + (++k));
			if(conf_load(para.conf)!=0) return (-1);
		}

		k++;
	}
	printf("\n");

	//加载配置文件参数
	boot_load();

	//启动加锁
	boot_try_lock();
	debug("boot_try_lock");

	//根据参数启动
	if(boot_dispatch())
		goto u_exit;

	debug("boot_dispatch");

	//启动服务
	boot_run();

	//资源回收
	u_exit:
		boot_release();
		boot_unlock();

	debug("boot_release");
	//退出
	return(0);
}
