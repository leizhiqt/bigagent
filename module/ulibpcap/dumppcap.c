#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <pcap.h>
#include "ulog.h"
//#include "times.h"
#include "dumppcap.h"

#define CF_PCAP_SIZE 1024*1024*10	//10M

// 会话句柄
static	pcap_t *handle;

//抓包线程
//抓包后处理方法
void loop_callback(u_char *args, const struct pcap_pkthdr *header, const u_char *packet) {
	//捕获数据包 并写入文件pcap
	//第一个参数是 pcap_dump_open() 打开的pcap_dumper_t* 类型数据， 需要手动转换为char *
	pcap_dump(args,header,packet);

	//按大小分割文件
	static u_int cf_size=0;

	cf_size += header->caplen;
	if(cf_size>=CF_PCAP_SIZE)
	{
		pcap_breakloop(handle);
		cf_size=0;
	}

	//
}

/*
*	开启抓包
*dev 网卡名称 执行嗅探的设备
*/
int promiscuous(const char *dev,const char *filter_rules,const char *save_path)
{
	//int k=-1;
	uid_t uid = getuid();
	if(uid!=0)
	{
		debug("promiscuous need root");
		return -1;
	}
	/*if(setuid(0))
	{
		return -1;
	}*/

	// 会话句柄
//	pcap_t *handle;

	//存储错误信息的字符串
	char errbuf[PCAP_ERRBUF_SIZE];
	memset(errbuf,'\0',sizeof(errbuf));

	//所在网络的掩码
	bpf_u_int32 mask;
	//主机的IP地址
	bpf_u_int32 net;

	// 探查设备属性
	pcap_lookupnet(dev, &net, &mask, errbuf); 
	//debug("%s",errbuf);

	// 以混杂模式打开会话
	handle = pcap_open_live(dev,65536,1,0,errbuf);
	//debug("%s",errbuf);

	//过滤器设置
	if(filter_rules!=NULL)
	{
		//snprintf(filter_rules,sizeof(filter_rules),"%s","");
		//snprintf(filter_rules,sizeof(filter_rules),"%s","port 80");

		//过滤器
		struct bpf_program filter;

		//编译并应用过滤器
		pcap_compile(handle,&filter,filter_rules,0,net);
		//过滤器设置
		pcap_setfilter(handle, &filter);
	}

	debug("promiscuous begin");
	//
	char ns[31];
	char pcap_full_path[128];
	while(1)
	{
		t_ftime((char *)&ns);

		memset(pcap_full_path,'\0',sizeof(pcap_full_path));

		if(save_path!=NULL && strlen(save_path)>0)
			strcat(pcap_full_path,save_path);

		if(strlen(pcap_full_path)>0)
			strcat(pcap_full_path,"/");

		strcat(pcap_full_path,ns);
		strcat(pcap_full_path,".pcap");

		debug("promiscuous write to %s",pcap_full_path);

		pcap_dumper_t *dumpfp=pcap_dump_open(handle,pcap_full_path);

		//设置回调函数
		pcap_loop(handle,-1,loop_callback,(u_char *)dumpfp);

		//刷新缓冲区
		pcap_dump_flush(dumpfp);
		//关闭pcap文件
		pcap_dump_close(dumpfp);
	}

	pcap_close(handle); /* 关闭会话 */

	//恢复uid
	/*if (setuid(uid))
	{

	}*/

	debug("promiscuous end");
	return(0);
}
