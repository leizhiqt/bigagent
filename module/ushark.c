#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>

#include "ulog.h"
#include "linklist.h"
#include "conf.h"
#include "times.h"
#include "ufile.h"

#include "capture_opts.h"

#include "ulibshark/ulibshark.h"
#include "uboot.h"

extern boot_parameters_t para;

static int pid;

static char *app;

void usage(char *p);

void boot_init()
{
	para_init();

	//注册信号量绑定
	//handler_int_1	接收中断--停止
	signal(SIGUSR1,boot_process_stop);
}

//中断	send_signal
void para_check()
{
	//pid文件不存在
	if(access(para.lock_file,F_OK|R_OK)==-1)
	{
		debug("%s 文件不存在",para.lock_file);
		if(para.status=='T' || para.status=='R'){
			exit(0);
		}
		return;
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

void para_load()
{
	if(!conf_enable()) return;

	logInit(conf_get("log_file"),0);

	para.branch_id = atoi(conf_get("branch"));
	snprintf(para.dev,sizeof(para.dev),"%s",conf_get("dev"));
	snprintf(para.save_pcap,sizeof(para.save_pcap),"%s",conf_get("save_pcap"));
	snprintf(para.filter_rule,sizeof(para.filter_rule),"%s",conf_get("filter_rule"));
	snprintf(para.lock_file,sizeof(para.lock_file),"%s",conf_get("pid"));
}

int boot_action()
{
	debug("boot_action %c",para.status);

	if(para.status=='S'||para.status=='R'){
		return 0;
	}

	if(para.status=='N'){
		usage(app);
		return -1;
	}

	if(para.status=='L'){
		ufile_list(para.read_pcap,para.branch_id);
		return -2;
	}

	return -3;
}

int boot_lock()
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

void usage(char *p)
{
	fprintf(stdout, "%s -s/-r -c agent.cnf\n",p);
	fprintf(stdout, "%s -t\n",p);

	fprintf(stdout, "%s -branch [1] -dev [enp0s31f6] -log [/var/tmp/ushark.log] -save [/var/tmp]\n",p);
	fprintf(stdout, "%s -branch 1 -dev enp0s31f6 -log /var/tmp/ushark.log -save /var/tmp\n\n",p);
	fprintf(stdout, "%s -branch 1 -dev enp0s31f6 -save /var/tmp\n",p);
	fprintf(stdout, "%s -branch 1 -log /tmp/ushark.log -rpcap /var/tmp\n",p);
}


void unlock()
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
}

int main(int argc,char **argv,char **envp)
{
	if (argc < 2)
	{
		usage(*argv);
		exit(1);
	}

	app = *argv;

	//初始化启动参数
	boot_init();

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
			if(conf_load(*(argv + (++k)))!=0) return (-1);
		}

		if (strcmp(argv[k],"-branch") == 0){
			para.branch_id = atoi(*(argv + (++k)));
			//printf("b1:%s\n",*(argv + (++k)));
			//printf("b2:%lu\n",branch_id);
		}

		if (strcmp(argv[k],"-dev") == 0){
			//dev = argv[++k];
			snprintf(para.dev,sizeof(para.dev),"%s",*(argv + (++k)));
		}

		if ((strcmp(argv[k],"-save") == 0)){
			snprintf(para.save_pcap,sizeof(para.save_pcap),"%s",argv[++k]);
			//创建目录
			mkdir_p(para.save_pcap,strlen(para.save_pcap));
		}

		if ((strcmp(argv[k],"-rpcap") == 0)){
			debug("-rpcap");
			para.status='L';
			snprintf(para.read_pcap,sizeof(para.read_pcap),"%s",argv[++k]);
			//printf("%s\n",para.read_pcap);
		}

		k++;
	}
	printf("\n");

	//加载配置文件参数
	para_load();
debug("para_load");

	//para_check
	para_check();
debug("para_check");
	//启动加锁
	boot_lock();
debug("boot_lock");
	//ushark初始化
	initialize(ushark_init,ushark_destroy,read_pcap);
debug("initialize");
	//根据参数启动
	if(boot_action())
		goto u_exit;
debug("boot_action");
	//ushark服务启动
	if(para.dev[0]!='\0')
		service_start(para.dev);
debug("service_start");
	//资源回收
	u_exit:
		release();
		unlock();

	//退出
	return(0);
}
