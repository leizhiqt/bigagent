#ifndef CF_PCAP_H
#define CF_PCAP_H

/*
pcap_file_header	文件头24B各字段说明:
magic_number:	4B	0x1A 2B 3C 4D:用来标示文件的开始
version_major:	2B	0x02 00:当前文件主要的版本号
version_minor:	2B	0x04 00当前文件次要的版本号
thiszone:	4B	当地的标准时间；全零
sigfigs:	4B	时间戳的精度；全零
snaplen:	4B	最大的存储长度
network:	4B	链路类型
				常用类型:
				0		BSD loopback devices, except for later OpenBSD
				1		Ethernet, and Linux loopback devices
				6		802.5 Token Ring
				7		ARCnet
				8		SLIP
				9		PPP
*/
typedef struct pcap_file_header pcap_file_header;

/*
Packet 包头和Packet数据组成
字段说明：
	ts.tv_usec		时间戳高位，精确到seconds
	ts.tv_se		时间戳低位，精确到microseconds
	caplen			当前数据区的长度，即抓取到的数据帧长度，由此可以得到下一个数据帧的位置。
	len				离线数据长度：网络中实际数据帧的长度，一般不大于caplen，多数情况下和Caplen数值相等。
					Packet 数据：即 Packet（通常就是链路层的数据帧）具体内容，长度就是Caplen，这个长度的后面，就是当前PCAP文件中存放的下一个Packet数据包，也就 是说：PCAP文件里面并没有规定捕获的Packet数据包之间有什么间隔字符串，下一组数据在文件中的起始位置。我们需要靠第一个Packet包确定
*/
typedef struct pcap_pkthdr pcap_pkthdr;

int cf_pcap_save(const char *save_pcap,const char *hexs,size_t size);

int cf_pcap_read(const char *cf_name);

int cf_boot_read(const char *cf_name,u_short branch_id);

//以太网帧最大长度
#define FRAME_BUFF_LEN 1500

void pcap_file_header_initialize(pcap_file_header *cf_header);

void pcap_pkthdr_initialize(pcap_pkthdr *pkthdr);

void print_pcap_file_header(pcap_file_header *pfh);

void print_pcap_pkthdr(pcap_pkthdr *ph);

void print_pcap(void * data,size_t size);

//按时间段分割文件 t_secs时间的前后n_secs秒分割为心pcap
int cf_pcap_split_tm(const char *cf_name,const time_t t_secs,const int n_secs);

#endif
