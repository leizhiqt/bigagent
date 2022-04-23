#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <time.h>

#include <pcap.h>

#include "ulog.h"
#include "linklist.h"
#include "ustring.h"
#include "ufile.h"
#include "cf_pcap.h"

/*
pcap_file_header	文件头24B各字段说明:
magic:			4B	0x1A 2B 3C 4D:用来标示文件的开始
version_major:	2B	0x02 00:当前文件主要的版本号
version_minor:	2B	0x04 00当前文件次要的版本号
thiszone:	4B	当地的标准时间；全零
sigfigs:	4B	时间戳的精度；全零
snaplen:	4B	最大的存储长度
linktype:	4B	链路类型
				常用类型:
				0		BSD loopback devices, except for later OpenBSD
				1		Ethernet, and Linux loopback devices
				6		802.5 Token Ring
				7		ARCnet
				8		SLIP
				9		PPP
*/
void pcap_file_header_initialize(pcap_file_header *cf_header)
{
	cf_header->magic=0xa1b2c3d4;
	//cf_header->magic=0xd4c3b2a1;
	cf_header->version_major=0x0002;
	cf_header->version_minor=0x0004;
	cf_header->thiszone=0x00;
	cf_header->sigfigs=0x00;
	cf_header->snaplen=0x00010000;
	cf_header->linktype=0x0001;
}

/*
Packet 包头和Packet数据组成
字段说明：
	ts.tv_sec		时间戳高位，精确到seconds
	ts.tv_usec		时间戳低位，精确到microseconds
	caplen			当前数据区的长度，即抓取到的数据帧长度，由此可以得到下一个数据帧的位置。
	len				离线数据长度：网络中实际数据帧的长度，一般不大于caplen，多数情况下和Caplen数值相等。
					Packet 数据：即 Packet（通常就是链路层的数据帧）具体内容，长度就是Caplen，这个长度的后面，就是当前PCAP文件中存放的下一个Packet数据包，也就 是说：PCAP文件里面并没有规定捕获的Packet数据包之间有什么间隔字符串，下一组数据在文件中的起始位置。我们需要靠第一个Packet包确定
*/
void pcap_pkthdr_initialize(pcap_pkthdr *pkthdr)
{
	pkthdr->ts.tv_sec=0x00;
	pkthdr->ts.tv_usec=0x00;
	pkthdr->caplen=0x00;
	pkthdr->len=0x00;
}

//int packet_to_pcap(void *packet,size_t size)
int cf_pcap_save(const char *save_pcap,const char *hexs,size_t size)
{
	//debug("packet_to_pcap");

	int k = strlen(hexs)/2;
	char FinalPacket[k+1];
	memset(FinalPacket,'\0',sizeof(FinalPacket));
	hex_to_binary(FinalPacket,hexs);

	pcap_file_header cf_header;
	pcap_pkthdr cf_pkthdr;

	pcap_file_header_initialize(&cf_header);
	pcap_pkthdr_initialize(&cf_pkthdr);

	cf_pkthdr.caplen=k;
	cf_pkthdr.len=k;

	//const char *save_pcap="test.pcap";

	binary_append(save_pcap,(const char *)&cf_header,sizeof(cf_header));
	//debug("cf_header=%d",sizeof(cf_header));

	binary_append(save_pcap,(const char *)&cf_pkthdr,8);
	binary_append(save_pcap,(const char *)&cf_pkthdr.caplen,4);
	binary_append(save_pcap,(const char *)&cf_pkthdr.len,4);

	//debug("cf_pkthdr=%d",sizeof(cf_pkthdr));

	binary_append(save_pcap,FinalPacket,k);
	//debug("caplen=%d",cf_pkthdr.caplen);

	//debug("caplen=%d",sizeof(cf_pkthdr.ts));
	//debug("caplen=%d",sizeof(cf_pkthdr.ts.tv_sec));
	//debug("caplen=%d",sizeof(cf_pkthdr.ts.tv_usec));
	//debug("caplen=%d",sizeof(cf_pkthdr.caplen));
	//debug("caplen=%d",sizeof(cf_pkthdr.len));

	return 0;
}

//读取数据报文件	cf_pcap_read [112.pcap]
int cf_pcap_read(const char *cf_name)
{
	debug("cf_read %s",cf_name);

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle;

	//handle=pcap_open_offline(fpcap,err);//读文件
	//p=pcap_dump_open(ohandle,fpcap);//读流
	//pcap_dumper_t *t = pcap_dump_open(dev, "./test.pcap");
	if((handle=pcap_open_offline(cf_name,errBuf))==NULL)
	{
		debug("read_pcap error: %s",errBuf);
		return -1;
	}
	//debug("read_pcap %p",handle);

	struct pcap_pkthdr *header;
	char *packet;

	int ret=0;
	//u_int k=0;
	while((ret=pcap_next_ex(handle,&header,(const u_char **)&packet)) > 0)
	{
		debug("read_pcap %d",ret);
		print_pcap_pkthdr(header);

		if(header->len > 0)
		{
			//第一个参数是 pcap_dump_open() 打开的pcap_dumper_t* 类型数据， 需要手动转换为char *
			//pcap_dump((char *)t, pkt, data);//写入文件

			/*for(int k=0;k<header->len;k++)
				printf("0x%02X",*(packet+k));
			printf("\n");*/
		}
	}

	pcap_close(handle);
	return 0;
}

int cf_boot_read(const char *cf_name,u_short branch_id)
{
	return cf_pcap_read(cf_name);
}

void print_pcap_file_header(pcap_file_header *pfh)
{
	if (pfh==NULL) {
		return;
	}
	printf("=====================\n"
			"magic:0x%0x\n"
			"version_major:%u\n"
			"version_minor:%u\n"
			"thiszone:%d\n"
			"sigfigs:%u\n"
			"snaplen:%u\n"
			"linktype:%u\n"
			"=====================\n",
			pfh->magic,
			pfh->version_major,
			pfh->version_minor,
			pfh->thiszone,
			pfh->sigfigs,
			pfh->snaplen,
			pfh->linktype);
}

void print_pcap_pkthdr(pcap_pkthdr *ph)
{
	if (ph==NULL) {
		return;
	}

	struct tm *tm_p = localtime(&ph->ts.tv_sec);

	debug("HEAD\tts.tv_usec:%u\tts.tv_sec:%u\tcaplen:%u\tlen:%d\t%04d-%02d-%02d",
			ph->ts.tv_usec,
			ph->ts.tv_sec,
			ph->caplen,
			ph->len,
			tm_p->tm_year+1900,tm_p->tm_mon+1,tm_p->tm_mday
			);
}

//按时间段分割文件 t_secs时间的前后n_secs秒分割为心pcap
//cf_name 源pcap文件
//t_secs 长整形的时间
//n_secs 前后多少秒
int cf_pcap_split_tm(const char *cf_name,const time_t t_secs,const int n_secs)
{
	debug("cf_read %s",cf_name);

	char errBuf[PCAP_ERRBUF_SIZE];
	pcap_t *handle;

	if((handle=pcap_open_offline(cf_name,errBuf))==NULL)
	{
		debug("read_pcap error: %s",errBuf);
		return -1;
	}
	//debug("read_pcap %p",handle);

	struct pcap_pkthdr *header;
	char *packet;

	pcap_dumper_t *p=NULL;
	p=pcap_dump_open(handle,"/tmp/up_tm.pcap");

	int ret=0;
	//u_int k=0;
	while((ret=pcap_next_ex(handle,&header,(const u_char **)&packet)) > 0)
	{
		debug("tv_sec %d",header->ts.tv_sec);
		if(header->len > 0 && header->ts.tv_sec >= t_secs-n_secs*60 && header->ts.tv_sec <= t_secs+n_secs*60)
		{
			//时间对比
			//写入文件 第一个参数是 pcap_dump_open() 打开的pcap_dumper_t* 类型数据， 需要手动转换为char *
			pcap_dump((unsigned char*)p,header,(u_char *)packet);
		}
	}

	pcap_close(handle);

	//关闭
	pcap_dump_close(p);

	return 0;
}

//pcap_ethernet解析	prinf_pcap
void print_pcap(void *data,size_t size)
{
	if (data==NULL) {
		return;
	}

	int i=0;
	u_char buf[FRAME_BUFF_LEN];

	u_char mac1[64];
	u_char mac2[64];
	snprintf(mac1,64,"%02X:%2X:%02X:%02X:%02X:%02X",*((u_char *)data),*((u_char *)data+1),*((u_char *)data+2),*((u_char *)data+3),*((u_char *)data+4),*((u_char *)data+5));
	snprintf(mac2,64,"%02X:%2X:%02X:%02X:%02X:%02X",*((u_char *)data+6),*((u_char *)data+7),*((u_char *)data+8),*((u_char *)data+9),*((u_char *)data+10),*((u_char *)data+11));

	debug("MAC:%s\t-->%s", mac1, mac2);//注意:printf函数自右向左求值、覆盖 

	struct in_addr addr1, addr2;
	char bf1[64];
	char bf2[64];

	//RFC 894
	ushort * tp = (ushort *)((u_char *)data+12);

	debug("tp->%04X",ntohs(*tp));

	ushort us;
	const char *pf = "ipv.txt";
	if(ntohs(*tp)==0x0800 || ntohs(*tp)==0x8006 || ntohs(*tp)==0x8035)
	{
		debug("RFC 894>>");
		//save_append("fpcap.txt","RFC 894\n");
		tp++;
		u_char *ucp = (u_char *)tp;
		u_char uc = *ucp;
		uc = uc >> 4;
		debug("version:\tipv%d",uc);

		snprintf(buf,sizeof(buf),"ipv%d\n",uc);
		save_txt(pf,(const char *)buf);

		uc = *ucp;
		uc = uc & 0x0f;
		debug("head size:\t%d (bytes)",uc*4);

		snprintf(buf,sizeof(buf),"%d (bytes)\n",uc*4);
		save_txt(pf,(const char *)buf);

		ucp++;
		uc = *ucp;
		debug("OTS:\t%02X",uc);

		snprintf(buf,sizeof(buf),"%02X\n",uc);
		save_txt(pf,(const char *)buf);

		ucp++;
		tp = (ushort *)ucp;
		us = *tp;
		debug("Packed size:\t%04X %d(bytes)",ntohs(us),ntohs(us));

		snprintf(buf,sizeof(buf),"%04X %d(bytes)\n",ntohs(us),ntohs(us));
		save_txt(pf,(const char *)buf);

		tp++;
		us = *tp;
		debug("Packed identifier:\t%04X",ntohs(us));

		snprintf(buf,sizeof(buf),"%04X\n",ntohs(us));
		save_txt(pf,(const char *)buf);

		tp++;
		us = *tp;
		ntohs(us);
		us = us >> 13;
		debug("Packed 3 bit:\t%02X",us);

		snprintf(buf,sizeof(buf),"%02X\n",us);
		save_txt(pf,(const char *)buf);

		us = *tp;
		ntohs(us);
		us = us & 0x1fff;
		debug("Packed 13 bit offset:\t%04X",us);
		snprintf(buf,sizeof(buf),"%04X\n",us);
		save_txt(pf,(const char *)buf);

		tp++;
		ucp = (u_char *)tp;
		uc = *ucp;
		debug("TTL:%d",uc);
		snprintf(buf,sizeof(buf),"%d\n",uc);
		save_txt(pf,(const char *)buf);

		ucp++;
		uc = *ucp;
		debug("network protocol type:\t%2X",uc);
		snprintf(buf,sizeof(buf),"%2X\n",uc);
		save_txt(pf,(const char *)buf);

		ucp++;
		tp = (ushort *)ucp;
		us = *tp;
		debug("16bit CRC:%04X",ntohs(us));

		snprintf(buf,sizeof(buf),"%04X\n",ntohs(us));
		save_txt(pf,(const char *)buf);

		//SIP->DIP
		tp++;
		ucp = (u_char *)tp;
		memcpy(&addr1, (char *)ucp, 4);//复制4个字节大小  
		memcpy(&addr2, (char *)ucp+4, 4);

		snprintf(bf1,64,"%s",inet_ntoa(addr1));
		snprintf(bf2,64,"%s",inet_ntoa(addr2));

		debug("IP:\t%s-->%s",bf1, bf2);
		snprintf(buf,sizeof(buf),"%s\t%s\n",bf1, bf2);
		save_txt(pf,(const char *)buf);

		debug("<<RFC 894");
	}
	else//802.3
	{
		debug("802.3");
		//save_append("fpcap.txt","802.3\n");
	}

	for (i=0; i<size;i++) {
		unsigned char ch = *((unsigned char *)data+i);
		if(i%6==0) printf("\n");
		printf("%02x",ch);
	}
	printf("\n");
}

//=========================================================
/*
//自定义 pcap文件读取并解析
int fpcap_analysis(const char *fpcap)
{
	fpcap_file_header  pfh;
	pcap_header  ph;
	int count=0;
	void * buff = NULL;
	int readSize=0;
	int ret = 0;

	printf("sizeof:int %lu,unsigned int %lu,char %lu,unsigned char %lu,short:%lu,unsigned short:%lu\n",
	sizeof(int),sizeof(unsigned int),sizeof(char),sizeof(unsigned char),sizeof(short),sizeof(unsigned short));

	logsk(1024);

	FILE *fp = fopen(fpcap, "rw");

	if (fp==NULL) {
		fprintf(stderr, "Open file %s error.",fpcap);
		ret = ERROR_FILE_OPEN_FAILED;
		goto ERROR;
	}
 
	fread(&pfh, sizeof(fpcap_file_header), 1, fp);
	prinf_fpcap_file_header(&pfh);
	//fseek(fp, 0, sizeof(pcap_file_header));
 
	buff = (void *)malloc(MAX_ETH_FRAME);
	for (count=1; ; count++) {
		memset(buff,0,MAX_ETH_FRAME);
		//read pcap header to get a packet
		//get only a pcap head count .
		readSize=fread(&ph, sizeof(pcap_header), 1, fp);
		if (readSize<=0) {
			break;
		}
		prinf_pcap_header(&ph);

		if (buff==NULL) {
			fprintf(stderr, "malloc memory failed.\n");
			ret = ERROR_MEM_ALLOC_FAILED;
			goto ERROR;
		}
 
		//get a packet contents.
		//read ph.capture_len bytes.
		readSize=fread(buff,1,ph.capture_len, fp);
		if (readSize != ph.capture_len) {
			free(buff);
			fprintf(stderr, "pcap file parse error.\n");
			ret = ERROR_PCAP_PARSE_FAILED;
			goto ERROR;
		}

		char packets[ph.capture_len+2];
		memset(packets,'\0',ph.capture_len+2);
		int k=0;
		for(int i=0;i<ph.capture_len;i++){
			u_char uc = *((u_char *)buff+i);

			if(isascii(uc) && !iscntrl(uc)){
				packets[k] = uc;
				k++;
				//printf("%c",uc);>>>>>>>>>>>>>>1
				//snprintf(packets,sizeof(packets),"%s%c",packets,uc);
			}
		}
		packets[k] = '\n';

		//snprintf(packets,sizeof(packets),"%s\n",packets);
		char *fstr = NULL;
		fstr = strstr(packets,"Test-group");
		int tk = -1;
		if(fstr != NULL){
			tk = fstr-packets;
		}

		debug("Test-group tk:%d",tk);
		if(tk>0){
			char bufpacks[strlen(packets)+1];
			memset(bufpacks,'\0',sizeof(bufpacks));
			memcpy(bufpacks,packets+tk,strlen(packets)-tk+1);

			//save_txt("ippacket.txt",bufpacks);

			fstr= strstr(bufpacks,";");
			tk = -1;
			if(fstr != NULL){
				tk = fstr-bufpacks;
			}

			if(tk>0){
				memset(packets,'\0',sizeof(packets));
				memcpy(packets,bufpacks,tk+1);
				packets[tk+1]='\n';

				save_txt("ippacket.txt",packets);
			}
		}
		debug("frame[%d],readSize:%d packload:%s",count,readSize,packets);

		prinf_pcap(buff, ph.capture_len);
		debug("frame[%d],readSize:%d buff:%s",count,readSize,buff);

		if (feof(fp) || readSize <=0 ) { 
			break;
		}
	}

ERROR:
	//free
	if (buff) {
		free(buff);
		buff=NULL;
	} 
	if (fp) {
		fclose(fp);
		fp=NULL;
	}	
	return ret;
}
*/

//一直读取文件更新
/*
void read_file(const char *path)
{
	int fd,n;
	int bk=0;
	char buf[1024];
	fd=open(path,O_RDONLY);

	debug("fd:%d",fd);

	while(1)
	{
		n=read(fd,buf,sizeof(buf));
		//debug("%d",n);
		if(n>0)
		{
			buf[n]='\0';
			debug("%s",buf);
		}
		else
		{
			bk++;
		}
		if(bk==100) usleep(10000);
	}
	close(fd);
}

//保存pcap bin
int m_write(const u_char *p,int len){
	FILE *fp;
	fp = fopen("pcap.bin","a+");
	fwrite(p,len,1,fp);
	fwrite("\n\n",4,1,fp);
	fclose(fp);
	
	int i=0;
	for(i=0;i<len;i++)
	{
		printf(" %02x",*(p++));
		if((i+1)%16==0){
			printf("\n");
		}
	}
	printf("\n");
}
*/

//捕获数据包 并写入文件pcap
/*
int write_pcap(pcap_t *handle,struct pcap_pkthdr *header,const u_char *packet)
{
	pcap_next_ex(handle,&header,(const u_char**)&packet);
	for(int k=0;k<header->len;k++)
		printf("%c",*(packet+k));
	printf("\n");
	
	pcap_dumper_t *p=NULL;
	p=pcap_dump_open(handle,"112.pcap");
	//写入
	pcap_dump((unsigned char*)p,header,packet);
	//关闭
	pcap_dump_close(p);
}
*/

