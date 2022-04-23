#ifndef U_BOOT_H
#define U_BOOT_H

//启动参数
typedef struct _boot_parameters{
	//分部ID
	u_short branch_id;
	//pcap文件路径
	char read_pcap[128];

	char dev[128];

	char save_pcap[128];

	char filter_rule[256];

	char status;

	char lock_file[128];
	
	int stop;
}boot_parameters_t;

//文件链表
typedef struct _pcap_node{
	link_node_t node;
	char buf[128];
}pcap_node_t;

//互斥量和条件变量
//pthread_mutex_t lock;

//启动参数初始化
void para_init();

//资源初始化
void initialize(int (*_fun_init) (),int (*_fun_destroy) (),int (*_fun_read_pcap) (const char *,u_short));

//资源释放
void release();

//多线程服务启动
void service_start(char *fpath);

//解析pcap文件存放的目录
void ufile_list(char *path,u_short branch_id);

static int *p;

//信号量处理方法
void boot_process_stop();

void pcap_breakloop_save();

#endif
