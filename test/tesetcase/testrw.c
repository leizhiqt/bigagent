/*读写锁

	pthread_rwlock_t rwlock_fd;

	//初始化读写锁
	pthread_rwlock_init(&rwlock_fd, NULL);
	//读锁
	pthread_rwlock_rdlock(&rwlock_fd);
	//{}
	pthread_rwlock_unlock(&rwlock_fd);
	//写锁
	pthread_rwlock_wrlock(&rwlock_fd);
	//{}
	pthread_rwlock_unlock(&rwlock_fd);
	//销毁
	pthread_rwlock_destroy(&rwlock_fd);
*/

/*信号量互斥锁

	pthread_mutex_t lock;

	//初始化互斥锁
	pthread_mutex_init(&lock,NULL);
	//加锁
	pthread_mutex_lock(&lock);
	//解锁
	pthread_mutex_unlock(&lock);
	//销毁
	pthread_mutex_destroy(&lock);
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <stddef.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ulog.h"
#include "dtype.h"

#include "thpool.h"
#include "linklist.h"

//每次最大数据传输量
#define MAXDATASIZE 2048

//链表节点
typedef struct _fpcap_node{
	link_node_t node;
	char buf[64];
}fpcap_node_t;
//通用链表
link_list_t link_list;

//遍历
int data_forth(link_list_t *link_list)
{
	link_node_t *p = link_list->head;

	fpcap_node_t *dp;
	while(p!=NULL){
		dp = container_of(p,fpcap_node_t,node);
		printf("data_forth data=%s p->next %p \n",dp->buf,p);

		p = p->next;
	}
	return 0;
}

//文件读写
#define FILE_LINE_LEN 1024

long g_curr_offset = 0;

int32_t c_tail(const char *file);

int32_t c_tail(const char *file)
{
	if (!file) return -1;
	FILE *fp = fopen(file, "r");
	if (!fp) {
		printf("cant open file, file: %s\n", file);
		return -2;
	}

	fseek(fp, g_curr_offset, SEEK_SET);

	char text[FILE_LINE_LEN];
	uint32_t len;
	while(!feof(fp)) {
		memset(text, 0x0, FILE_LINE_LEN);
		fgets(text, FILE_LINE_LEN, fp);
		len = strlen(text);
		if (len == 0 || text[len - 1] != '\n') continue;
		text[len - 1] = 0;
		g_curr_offset += len;
		printf("%s\n", text);
	}

	fclose(fp);

	return 0;
}
//
int test(const char *file_path)
{
	while(1){
		c_tail(file_path);
	}
	return 0;
}
//

//开辟线程池
static	thpool_t thpool;

static int max_cases;

static long count=0;

//客户端
typedef struct _client client_t;

typedef struct _client{
	int fd;
	//线程读写锁
	pthread_rwlock_t rwlock_fd;

	uchar buf[MAXDATASIZE];
	int buf_size;
}client_t;

//test_case_read
void *test_case_read(void *v)
{
	int k=-1;
	
	client_t* client = (client_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	debug("test_case_read 开始:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	
	while(1)
	{
		pthread_rwlock_rdlock(&(client->rwlock_fd));
		//debug("pthread_rwlock_rdlock:%d",k);
		printf("=========================\n");
		data_forth(&link_list);
		//printf("test_case_read:%lu\n",count);

		pthread_rwlock_unlock(&(client->rwlock_fd));

		sleep(1);
	}

	sleep(1);
	debug("test_case_read 结束:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
}

//test_case_write
void *test_case_write(void *v)
{
	int k=-1;

	client_t* client = (client_t *)v;

	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	debug("test_case_write 开始:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	
	while(1)
	{
		k=pthread_rwlock_wrlock(&(client->rwlock_fd));
		//debug("pthread_rwlock_wrlock:%d",k);
		//count++;
		fpcap_node_t *fpcap_node_p=NULL;
		fpcap_node_p= (fpcap_node_t*) malloc(sizeof(fpcap_node_t));
		
		//snprintf(link_data->buf,sizeof(link_data->buf),"%s%d","str",i);
		link_list_add_last(&link_list,&fpcap_node_p->node);
		//printf("link_data_t add1 %p\n",&(link_data->node));
		
		pthread_rwlock_unlock(&(client->rwlock_fd));

		//usleep(2);
		sleep(2);
	}

	sleep(1);
	debug("test_case_write 结束:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	//pthread_exit(NULL);
}

void test_init(client_t	*client)
{
	int k=-1;
	
	max_cases = 10;
	//初始化读写锁
	k=pthread_rwlock_init(&(client->rwlock_fd), NULL);
	
	debug("%d",k);
	
	//线程池
	thpool_init(&thpool,75,5);
	usleep(50);
	
	//初始化队列
	link_list_init(&link_list);
}

void test_destroy(client_t *client)
{
	max_cases = 0;
	pthread_rwlock_destroy(&(client->rwlock_fd));
	
	thpool_destroy(&thpool);
	//printf("thpool_destroy\n");
	
	//释放化队列
	link_list_destroy(&link_list,fpcap_node_t);

	//等待其他线程执行完
	usleep(1000);
}

void test_start(client_t* client)
{
	if(thpool_add_work(&thpool,test_case_read,client)<0)
	 {
		 debug("thpool full");
	 }
	sleep(3);
	
	if(thpool_add_work(&thpool,test_case_write,client)<0)
	 {
		 debug("thpool full");
	 }
	sleep(3);

	debug("start_test finsh");
}

//=======================
//文件list
void f_list(char *path,boolean is_down)
{
	DIR		*pDir; 
	struct dirent	*ent; 
	int		i=0; 
	char	childpath[512]; 

	pDir=opendir(path); 
	memset(childpath,0,sizeof(childpath));

	while((ent=readdir(pDir))!=NULL)
	{ 
		if(ent->d_type & DT_DIR)
		{ 
			if(strcmp(ent->d_name,".")==0 || strcmp(ent->d_name,"..")==0)
				continue; 

			//递归下级目录
			if(is_down)
			{
				sprintf(childpath,"%s/%s",path,ent->d_name);
				printf("path:%s\n",childpath);

				f_list(childpath,is_down);
			}
		}
		else
		{
			sprintf(childpath,"%s/%s",path,ent->d_name);
			printf("%s\n",childpath);
			//
		}
	}
	//
	closedir(pDir);
}

//文件夹监控
int directory_monitor(const char *file_path)
{
	return 0;
}

void usage()
{
	fprintf(stdout, "\ttestrw -n10\n");
}

int main(int argc, char** argv, char *envp[]){

	if (argc < 1)
	{
		usage();
		exit(1);
	}

	client_t client;

	char buf[64] = {0};
	int k = 0;
	while (argc > k) {
		//printf("%s\n",argv[k]);
		if (strstr(argv[k],"-n") == argv[k]){
			debug("-n%s",argv[k]+2);
			max_cases = atoi(argv[k]+2);
		}

		k++;
	}
	printf("\n");
	
	test_init(&client);//参数初始化

	//启动测试用例
	test_start(&client);
	
	//顺序测试
	//f_list("/home/zlei/example-mysql",false);
	
	test_destroy(&client);
	
	printf("testrw exit\n");
	
	return 0;
}
