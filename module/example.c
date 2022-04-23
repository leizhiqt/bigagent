//#define _XOPEN_SOURCE
//#define _BSD_SOURCE
//#define _SVID_SOURCE
//#define _XOPEN_SOURCE

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

//debug("master_enable %s",master_enable);
/*if(!strcmp(master_enable,"Yes"))
{
	if(fork() == 0)
	{
		//child process
		char *execvp_argv[] = {"/opt/agent/bin/fserver",">","/opt/agent/fserver.log", "2>&1","&",NULL};
		if (execvp("/opt/agent/bin/fserver",execvp_argv) <0 ){
			error("/opt/agent/bin/fserver error");
		}
		exit(0);
	}
}*/

//#define _XOPEN_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/time.h>

#include <fcntl.h>
#include <pthread.h>

#include <netdb.h>
#include <sys/socket.h>

#include <unistd.h>
#include <sys/types.h>

#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stddef.h>

#include <pcap.h>

#include "ulog.h"
#include "dtype.h"
#include "linklist.h"

#include "conf.h"
#include "thpool.h"
#include "taskdata.h"
#include "csocket.h"

#include "times.h"
#include "ustring.h"
#include "ufile.h"

#include "ethernet.h"
#include <mysql.h>
#include "mysqlconn.h"
#include "db_ethernet.h"
#include "db_service.h"

#include "ulibpcap/cf_pcap.h"
#include "extra/forward.h"
#include "extra/forward_udp.h"
#include "extra/slave.h"

/*
#include "stdio.h"
#include "stdlib.h"
#include "errno.h"
#include "string.h"
#include "time.h"
#include "unistd.h"
#include "fcntl.h"
#include "signal.h"

#include "pthread.h"

#include "sys/time.h"
#include "sys/types.h"
#include "sys/socket.h"
#include "sys/wait.h"
#include "dirent.h"

#include "linux/ip.h"

#include "mcheck.h"

#include "netinet/in.h"
#include "arpa/inet.h"
#include "asm/byteorder.h"

#include "zlib.h"

#include "cv.h"
#include "cxcore.h"
#include "highgui.h"

#include "times.h"
#include "ulog.h"
#include "thpool.h"
#include "linklist.h"
#include "websocket.h"
#include "sfline.h"
*/

typedef struct _st
{
	int a;
	int b;
}st_t;

static st_t st;

void swap_value(int x,int y)
{
	int temp;
	temp = x;
	x = y;
	y = temp;   
	printf("传值函数内的输出 %d %d \n",x,y);
}

void swap_address(int *x,int *y)
{
	int temp;
	temp = *x;
	*x = *y;
	*y=temp;   
	printf("传址函数内的输出 %d %d \n",*x,*y);
}

void change(int **x){
	*x = malloc(sizeof(int));
	**x = 300;
	
	printf("传址函数内的输出 %p %p %d\n",x,*x,**x);
}

void change_val(int *x){
	int *p =x;
	*x = 234;
	//printf("传址函数内的输出 %p %p %d\n",x,*x,**x);
}

void change2(char *s,int size){
	snprintf(s,size,"%s","987654321");
}

void change1(char *s,int size){
	snprintf(s,size,"%s","123456789");
	change2(s,size);
}

char *(strcat)(char *s1, const char *s2)
{
	char *s;
	for(s=s1;*s!='\0';++s);
	for(;(*s=*s2)!='\0';++s,++s2);
	return (s1++);
}

/*
int *(function)(int *ret)
{
	char buffer[4];//4 bytes 8bytes
	//memset(buffer,0,4);
	//int ax=100;
	//int *ret;//4 bytes 8bytes
	//rbp 4 bytes=12  8bytes =24
	ret = (int*)(&buffer + 24); //64bits 24  32bits 12
	(*ret) += 7; //x86=7 arm=4

	register int a = 88;

	__asm__ __volatile__("\
		movl $1,%eax\n\
		xor %rbx,%rbx\n\
		int $0x80"
		);

	__asm__ __volatile__(
		"movl $10,%%eax\n"
		"movl $0x10,%%eax\n"
		//"mov %%rip,%%rax\n"
		:"=a"(a)
		);
	printf("a = %d\n",a);

	return (ret);
}

int fx() { 
	int x; 
	x = 20;
	int *p;
	function(p);//push rip
	x = 1;
	printf("x=%d\n",x); 
}
*/

/*int ch1(match_video_t *p) { */
/*	p->matching=5555.55;*/
/*	printf("ch1 x=%0.4lf\n",p->matching); */
/*}*/

/*int ch(match_video_t *p) { */
/*	p->matching=1234.55;*/
/*	//ch1(p);*/
/*	printf("ch x=%0.4lf\n",p->matching); */
/*}*/

int example_malloc() {
	char *s1 = "aaaaa";
	char *s2 = "bbbbbbbbb";
	//char **s ={"aaaaa","bbbbb","ddddd"};
	char **s = malloc(sizeof(char **));
	*s = s1;
	*(s+1) = s2;
	printf("%s %s \n",*s,*(s+1));
	return 0;
}

void display(void *v)
{
	int *p = (int *)v;
	printf("main x=%d %d %d\n",*p,*(p+1),*(p+2));
}

int setdate(char *s0,char *s1)
{
	struct tm tm0;
	struct tm tm1;

	char buf[255];

	memset(&tm0, 0, sizeof(struct tm));
	strftime(s0,8,"%Y%m%d", &tm0);

	memset(&tm1, 0, sizeof(struct tm));
	strftime(s1,8, "%Y%m%d", &tm1);
	
	strftime(buf,8, "%Y%m%d", &tm0);
	strftime(buf,8, "%Y%m%d", &tm1);
	
	long unixtime0 = mktime(&tm0);
	long unixtime1 = mktime(&tm1);

	//time_t curTime;
	//time(&curTime);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm0);

	printf("%s %ld %ld\n",buf,unixtime0,unixtime1);

	return 0;
}

void m_malloc(){
	int i,j;
	i=9;
	j=5;
	char ch1[i][j];
	
	char **pp = malloc(sizeof(char *)*i);
	for(int i1=0;i1<i;i1++){
		*(pp+i1) = malloc(sizeof(char)*(i1+1));
		snprintf(*(pp+i1),i1+1,"%s","abcdefghijklmn");
	}

	for(int i1=0;i1<i;i1++){
		printf("%s\n",*(pp+i1));
		free((*(pp+i1)));
	}
	
	free(pp);
/*
	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			ch1[i1][j1]=i1*10+j1;
		}
	}

	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			printf("%d %d %d\n",i1,j1,ch1[i1][j1]);
		}
	}

	printf("================\n");
	char (*p)[j] = ch1;

	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			printf("%d %d %d\n",i1,j1,*(*(p+j1)+i1));
		}
	}*/

	/*printf("================\n");
	char (*p1)[i][j] = &ch1;
	for(int i1=0;i1<i;i1++){
		for(int j1=0;j1<j;j1++){
			printf("%d %d %d\n",i1,j1,*(*(p1+j1)+i1));
		}
	}*/

	//printf("传址函数内的输出 %p %p %d\n",x,*x,**x);
}

void nprint(){
	char s1[128];
	snprintf(s1,sizeof(s1),"%s/%s","100","1000");
	printf("%s\n",s1);
}
void ttoken(){
	char buf[]=": : : : : :";
	char *token=strtok(buf,":");
	int k=0;
	debug(" recv_cmd ERROR k:%d token:%s",k,token);
	while(token!=NULL){
		debug(" recv_cmd ERROR k:%d token:%s",k,token);
		token=strtok(NULL,":");
		k++;
	}
}

void tstrstr(){
	char buf[]="12.jpg";
	char *p=strstr(buf,".jpg");
	debug(" tstrstr:%s",p);
}

typedef struct _people
{
	int age;
	char name[64];
}people_t;

void *case_start(void *v)
{
	//int k = *(int *)v;
	
	people_t people = *(people_t *)v;
	
	pthread_t pid = getpid();
	pthread_t tid = pthread_self();
	
	debug("case_start 开始:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
	
	while(1)
	{
		debug("case_start 运行:k:%d pid:%u pthread_id:%u (0x%x)",people.age,(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
		sleep(1);
	}
	debug("case_start 结束:pid:%u pthread_id:%u (0x%x)",(unsigned int)pid,(unsigned int)tid,(unsigned int)tid);
}

//01:102:18,test4:02
//01:104:10,fulltest:0201:102:18,test4:02
void send_plc(char* cmd,int c_size)
{
	client_t client;
	client.host=gethostbyname("192.168.1.50");
	client.port=4196;

	memset(client.buf, '\0',sizeof(client.buf));
	client.buf_size=c_size;

	memcpy(client.buf,cmd,c_size);

	short_send(&client);
}

void open_plc()
{
	//01 05 00 10 ff 00 8d ff	开
	//01 05 00 10 00 00 cc 0f	关
	char open_plc[] = "\x01\x05\x00\x10\xff\x00\x8d\xff";
	char close_plc[] = "\x01\x05\x00\x10\x00\x00\xcc\x0f";

	send_plc(open_plc,8);

	sleep(1);
	send_plc(close_plc,8);
}

typedef struct _example_node{
	link_node_t node;
	char buf[64];
}example_node_t;

int data_forth(link_list_t *link_list)
{
	link_node_t *p = link_list->head;

	example_node_t *example_node_p;
	while(p!=NULL){
		example_node_p = container_of(p,example_node_t,node);
		printf("data_forth data=%s p->next %p \n",example_node_p->buf,p);

		p = p->next;
	}
	return 0;
}

void example_link_list() {
	link_list_t link_list;
	link_list_init(&link_list);

	//printf("check mem1\n");
	//sleep(30);

	example_node_t *example_node_p=NULL;
	for(int i=0;i<5;i++){
		example_node_p= (example_node_t*) malloc(sizeof(example_node_t));
		snprintf(example_node_p->buf,sizeof(example_node_p->buf),"%s%d","str",i);
		link_list_add_last(&link_list,&example_node_p->node);
		printf("link_data_t add1 %p\n",&(example_node_p->node));
	}
	data_forth(&link_list);

	//printf("check mem2\n");
	//sleep(30);
	printf("link_data_t size %d\n",link_list_size(&link_list));

	link_list_remove_first(&link_list,example_node_t);
	printf("link_data_t size %d\n",link_list_size(&link_list));

	link_list_remove_last(&link_list,example_node_t);
	printf("link_data_t size %d\n",link_list_size(&link_list));

	link_list_remove_last(&link_list,example_node_t);
	printf("link_data_t size %d\n",link_list_size(&link_list));

	data_forth(&link_list);

	link_list_destroy(&link_list,example_node_t);
	
	//printf("check mem3\n");
	//sleep(30);
	
	//data_forth(&link_list);
}

void example_1() {
	int k = exist_line("data/137/1/1.txt","2017-03-01 17:07:13");
	printf("main x=%d\n",k);
	k = exist_line("data/137/1/1.txt","2017-03-01 17:07:08");
	printf("main x=%d\n",k);
}

/*
void example_2() {
	char *s = "HTTP/1.1 101 Switching Protocols\
Upgrade: websocket\
Connection: Upgrade\
Sec-WebSocket-Accept: bUhrHw==";
	int k = recv_protocol(s);
	printf("recv_protocol=%d\n",k);
}
*/

void example_3() {
	//tarz("data/74/3/","data/74/3.tar.gz");
	//gzip_d("test","test.zip");
}
/*
void example_4() {
	char buf[80];
	set_save_path((char *)&buf);
	printf("task_flush=%s\n",buf);
}
*/
void example_5() {
	char *path = "data/tmp/abcd";
	mkdir_p(path,strlen(path));
}

void example_6() {
	task_init(10);
	int k = task_flush("abcd");
	printf("task_flush=%d\n",task_exist("abcd3"));
	printf("task_flush=%d\n",task_exist("abcd"));
	k=task_flush("abcd");
	printf("task_flush=%d\n",k);
	k=task_flush("abcd");
	printf("task_flush=%d\n",k);
	k=task_flush("abcd3");
	printf("task_flush=%d\n",k);
	k=task_flush("abcd5");
	printf("task_flush abcd5=%d\n",task_exist("abcd3"));

	printf("task1=%d\n",task_size());
	task_remove("abcd3");
	task_remove("abcd5");
	task_remove("abcd");
	task_remove("abcd");
	printf("task2=%d\n",task_size());
	task_destroy();
	printf("task3=%d\n",task_size());
}

void example_7() {
	char ch[] = "01:104:10,fulltest:02\n\rbbb\n\r";
	debug(" received analyData:%s< %d",ch,strlen(ch));
	brline(ch,strlen(ch));
	debug(" received analyData:%s< %d",ch,strlen(ch));
}

void example_8() {
	//debug(" received analyData:%d",recv_protocol(ch));
	//char *ch = "01:101:154,82,288,254:02";
	//char *ch = "01:100:rrrr:02";
	//recv_cmd(ch);
}

void example_9() {
	char buf[] = "abcd0";
	char *p = buf;
	for(int i=0;i<sizeof(buf);i++)
	debug(" received analyData:0x%02X",*(p+i));
}
/*
void example_10() {
	task_match_t task_match;
	task_match.matching = 999.999;
	ch(&task_match);
	debug(" p->matching:%0.4lf",task_match.matching);
}

void example_11() {
	int i[3]={'2','3','4'};
	task_match_t task_match;
	
	printf("main x=%d\n",sizeof(task_match.msg));
}
*/

void example_12() {
	//printbytes((char *)&i,3);
	int i[3]={2,3,4};
	display(i);
	printf("main x=%d %d %d\n",i[0],i[1],i[2]);
}

void example_13() {
	//char *fname = ex();
	//printf("main fname=%s\n",fname);
	int x=90;
	int *p = &x;
	change_val(p);
	printf("main x=%d\n",x);
}
/*
void example_14() {
	task_match_t mmatch;
	mmatch.matching = 20.1234;
	printf("main x=%0.4lf\n",mmatch.matching); 
	ch(&mmatch);
	printf("main x=%0.4lf\n",mmatch.matching);
}
*/

void example_15() {
	char buf[64];
	char s1[] = "NO. warning!!!!";
	snprintf(buf,3,"%s","NO. warning!!!!");
	printf("buf:%s\n",buf);
}

void example_16() {
	//printf("main x=%d\n",abs(2,3));
	int x = 1;
	int y = 2;

	printf("x y \n");
	printf("初值 %d %d \n",x,y);
	//传值子程序调用(交换xy) 
	swap_value(x,y);
	printf("传值函数外调用 %d %d \n",x,y);

	int* x1=NULL;
	
	printf("传值函数外调用 %p \n",x1);
	change(&x1);
	printf("传值函数外调用 %p %d\n",x1,*x1);
	
	//传地址字程序调用(交换x,y) 
	swap_address(&x,&y);
	printf("传址函数外调用 %d %d \n",x,y);
	
	int *p = &x;
	change_val(p);
	printf("传址函数外调用 %d %d \n",x,y);
	//fx();
}
void example_17() {
	//线程池
	thpool_t thpool;
	
	thpool_init(&thpool,25,5);
	//usleep(50);
	sleep(1);
	
	people_t people;
	
	for(int i=0;i<25;i++)
	{
		 debug("test_case[%d]",i);
		 people.age=i;
		 
		 if(thpool_add_work(&thpool,case_start,&people)<0)
		 {
			 debug("thpool full");
		 }
		 sleep(1);
	}
	
	printf("thpool_destroy\n");
	thpool_destroy(&thpool);

	//等待其他线程执行完
	sleep(1);
	printf("Main App exit\n");
}

void example_18() {
	struct tm tm;
	char buf[255];

	memset(&tm, 0, sizeof(struct tm));
	strftime("2017-11-12 18:31:01",20, "%Y-%m-%d %H:%M:%S", &tm);
	//strftime(buf, sizeof(buf), "%d %b %Y %H:%M", &tm);
	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
	long unixtime = mktime(&tm);

	time_t curTime;
	time(&curTime);
	printf("%s %ld %ld\n",buf,unixtime,curTime);
}

void example_19() {
	char buf[8];
	change1(buf,sizeof(buf)-1);
	debug("%s",buf);
}

void example_20() {
	char buf[8];
	change1(buf,sizeof(buf)-1);
	debug("%s",buf);
}

//IP地址转换
//char ip1[] = "192.0.1.130";
//char ip2[] = "192.0.1.57";
int ipaddr(const char *ip1,const char *ip2)
{
	struct in_addr addr1, addr2;
	long l1, l2;  
	l1 = inet_addr(ip1);   //IP字符串——》网络字节  
	l2 = inet_addr(ip2);  
	printf("IP1: %s \t IP2: %s\n", ip1, ip2);  
	printf("l1: %ld \t l2: %ld\n", l1, l2);  

	printf("Addr1: %08lX \t Addr2: %08lX\n", l1, l2);

	memcpy(&addr1, &l1, 4); //复制4个字节大小  
	memcpy(&addr2, &l2, 4);  
	//printf("%s <--> %s\n", inet_ntoa(addr1), inet_ntoa(addr2)); //注意：printf函数自右向左求值、覆盖
	char bf1[64];
	char bf2[64];
	snprintf(bf1,64,"%s",inet_ntoa(addr1));
	snprintf(bf2,64,"%s",inet_ntoa(addr2));

	printf("%s \t %s\n",bf1,bf2); //网络字节 ——》IP字符串
	//printf("%s\n", inet_ntoa(addr2));
	return 0;
}

//
void ustring_1()
{
	//goose pack
	char *payload="010ccd01000140618672b81f8100800088b803e800c1000000006181b6802973696d"
			"706c65494f47656e65726963494f2f4c4c4e3024474f24676362416e616c6f6756616c7565738101008223736"
			"96d706c65494f47656e65726963494f2f4c4c4e3024416e616c6f6756616c756573832973696d706c65494f47656e657"
			"26963494f2f4c4c4e3024474f24676362416e616c6f6756616c756573840859dc880df3f7ce0a85010186010087010088010189010"
			"08a0104ab1a850204d28c060000000000008502162e8a086161616161616161";

}

void example_debug()
{
	//ustring_1();
	int k=4095;
	char buf[k];

	for(int i=0;i<k;i++){
		buf[i]='A';
		//printf("%c",buf[i]);
	}
	buf[k-1]='\0';
	//printf("\n");

	printf("%s\t%d\t[DEBUG]:%s\n",__FILE__,__LINE__,buf);

	char *ft = "%sBB";
	debug(ft,buf);

}

void example_forward_udp()
{
	addr_node_t **udp_pool = (addr_node_t **) malloc(sizeof(addr_node_t *)*2);

	//172.25.6.110:5000
	addr_node_t *addr_node = NULL;

	addr_node = (addr_node_t *) malloc(sizeof(addr_node_t));
	snprintf(addr_node->ip_addr,sizeof(addr_node->ip_addr),"%s","172.25.6.110");
	addr_node->port=5000;
	*(udp_pool+0) = addr_node;

	addr_node = (addr_node_t *) malloc(sizeof(addr_node_t));
	snprintf(addr_node->ip_addr,sizeof(addr_node->ip_addr),"%s","172.25.6.90");
	addr_node->port=1234;
	*(udp_pool+1) = addr_node;

	set_forward_pool((const addr_node_t **)udp_pool,2);

	int k=1024;//k
	char buf[k];

	for(int i=0;i<k;i++){
		buf[i]='A';
		//printf("%c",buf[i]);
	}
	//buf[k-1]='\0';

	long a=1000000000000;
	
	ustring *u_str=NULL;
	for (long i=0;i<a;i++){
		u_str = ustring_new();
		for(int j=0;j<32;j++){
			//debug("buf=%s len=%d",buf,k);
			ustring_append(u_str,buf,k);
			//ustring_append(u_str,"\n",1);
			//printf(">>>>>>>>>>>>>>>>>>>\n");
		}
		ustring_append(u_str,"\r\n",2);

		char *ptr = ustring_rebuild(u_str);
		debug("%ld",i);
		forward_send(ptr,strlen(ptr));

		ustring_free(u_str);
		usleep(300);
	}
}

void example_mysql() {
	mysql_conn_init(mysql_conn_get_default());
/*	mysql_conn_query(&mysql_conn,"SELECT id, my_name FROM affected_rows");*/
/*	//mysql_conn_execute(&mysql_conn,"SELECT 1");*/
/*	mysql_conn_query(&mysql_conn,"SELECT 1");*/
/*	mysql_conn_query(&mysql_conn,"SELECT 12");*/

	//db_ethernet_init();

	db_ip_t db_ip;
	db_ip_init(&db_ip);
	db_ip.branch_id=100;

	int k=4096;//k
	char buf[k];

	for(int i=0;i<k;i++){
		buf[i]='A';
		//printf("%c",buf[i]);
	}

	db_ip.payload=buf;
	db_ip.payload_len=k;
	static long c=0;
	while(1){
		db_ethernet_add(&db_ip);
		debug("%ld",c++);
	}
	mysql_conn_release(mysql_conn_get_default());
}

void ustring_example() {
	int k=1024;
	int k10=10*k;
	int k11=k10-1;

	char buf[10*k];
	memset(buf,'A',k11);

	char *ptr;
	ustring *ustring;
	int n=0;
	while(1)
	{
		ustring = ustring_new();
		ustring_append(ustring,"{",1);
		ustring_append(ustring,buf,k11);
		ustring_append(ustring,"}",1);

		ptr = ustring_rebuild(ustring);
		debug("ustring->size %d",ustring->size);

		ustring_free(ustring);

		if(n++==3) break;

		//usleep(3000);
	}
	//sleep(5);
}

void example_uconf()
{
	link_list_t *u_conf=uconf_new();

	const char *conf_path = "/home/zlei/project-c/c-build/bigagent/conf/test.cnf";

	uconf_load(u_conf,conf_path);

	const char *value=uconf_get(u_conf,"test");
	debug("test=%s",value);

	int size = link_list_size(u_conf);
	debug("size=%d",size);

	value=uconf_get(u_conf,"abcd");
	debug("test=%s",value);

	uconf_destroy(u_conf);
	//u_conf=NULL;

	size = link_list_size(u_conf);
	debug("size=%d",size);
}

void example_cf_pcap(const char *cf_name)
{
	cf_pcap_split_tm(cf_name,1567160466,1);
}

int main(int argc, char **argv,char **envp)
{
	/*log_set_level(1);

	debug("ustring_cf_pcap");
	example_cf_pcap(argv[1]);*/

	const char *s_format="%b %d, %Y %H:%M:%S          %Z";
	const char *d_format="%Y-%m-%d %H:%M:%S";

	const char *ts="Nov 27, 2019 12:54:51.181780402 CST";

	//const char *s_format="%b %d, %Y %H:%M:%s";
	//const char *ts="Nov 27, 2019 12:54:51";

	//const char *s_format="%Y-%m-%d %H:%M:%S";
	//const char *ts="1990-11-07 08:01:51";
	char buf[21];

	t_format_tt(ts,s_format,d_format,buf,20);

	debug("d_times:%s",buf);

	//debug("t_format_tt");

	//debug("ustring_example");
	//ustring_example();

	//debug("example_uconf");
	//example_uconf();

	//debug("example_forward_udp");
	//example_forward_udp();

	//debug("example_mysql");
	//example_mysql();

	return 0;
}
