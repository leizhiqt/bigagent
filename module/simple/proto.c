#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>

#include <dirent.h>

#include <time.h>
#include <sys/time.h>

#include <sys/socket.h>
#include <errno.h>

#include <pcap.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ulog.h"
#include "linklist.h"
#include "ustring.h"
#include "ufile.h"
#include "dtype.h"
#include "ethernet.h"
#include "db_ethernet.h"
#include "db_service.h"
#include "proto.h"
#include "review.h"

//pcap_frame
int pcap_frame(const struct pcap_pkthdr *header,db_ip_t *db_ip)
{
	struct tm tm0;
	char buf[64];

	strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime_r(&(header->ts.tv_sec), &tm0));
	snprintf(db_ip->frame_tv,sizeof(db_ip->frame_tv),"%s",buf);
	snprintf(db_ip->frame_caplen,sizeof(db_ip->frame_caplen),"%u",header->caplen);
	snprintf(db_ip->frame_len,sizeof(db_ip->frame_len),"%u",header->len);

	debug("datetime:%s caplen:%u len:%u",db_ip->frame_tv,header->caplen,header->len);

	debug("datetime:%s caplen:%s len:%s",db_ip->frame_tv,db_ip->frame_caplen,db_ip->frame_len);
	
	return 0;
}

//pcap_ethernet_rfc894
int pcap_ethernet_rfc894(u_short type)
{
	switch(type)
	{
		case 0x0800:
			debug("Ethernet RFC894 IP:%04X",type);
			break;
		case 0x0806:
			debug("Ethernet RFC894 ARP:%04X",type);
			break;
		case 0x8035:
			debug("Ethernet RFC894 ARP:%04X",type);
			break;
		default:
			debug("Ethernet RFC1042 802.2/802.3 len:%04X %d",type,type);
			break;
	}
	return 0;
}

int pcap_ethernet_8023(const u_char *packet)
{
	u_char *p= (u_char *)packet+14;
	
	u_char dsap=*p;
	u_char ssap=*(p+1);
	u_char cntl=*(p+2);
	u_int org_code = ( *(p+3)<<16 | *(p+4)<<8 | *(p+5) ) | 0x00ffffff;
	u_short type = *(p+6)<<8 | *(p+7);
	
	debug("DSAP %02x SSAP %02x CNTL %02x ORG_CODE %06x TYPE %04x",dsap,ssap,cntl,org_code,type);
	pcap_ethernet_rfc894(type);
	return 0;
}

//pcap_ethernet解析 基于libpcap
int pcap_ethernet(const u_char *packet,db_ip_t *db_ip)
{
	// 以太网头
	//const struct packet_ethernet *ethernet;	// The ethernet header [1]
	struct packet_ethernet *ethernet;	// The ethernet header [1]
	ethernet = (struct packet_ethernet*)(packet);

	u_char mac_src[18];
	u_char mac_dst[18];

	u_char *mac_byte;
	mac_byte = (u_char *)&(ethernet->ether_mac_src);
	snprintf(mac_src,18,"%02x:%02x:%02x:%02x:%02x:%02x",*(mac_byte),*(mac_byte+1),*(mac_byte+2),*(mac_byte+3),*(mac_byte+4),*(mac_byte+5));
	snprintf(db_ip->eth_src,sizeof(db_ip->eth_src),"%s",mac_src);
	
	mac_byte = (u_char *)&(ethernet->ether_mac_dst);
	snprintf(mac_dst,18,"%02x:%02x:%02x:%02x:%02x:%02x",*(mac_byte),*(mac_byte+1),*(mac_byte+2),*(mac_byte+3),*(mac_byte+4),*(mac_byte+5));
	snprintf(db_ip->eth_dst,sizeof(db_ip->eth_dst),"%s",mac_dst);

	u_short type = HL_LH(ethernet->ether_type);

	debug("Ethernet MAC src:%s\tMAC dst:%s", mac_src, mac_dst);
	//>0x05DC
	//type==0x0800 || type==0x0806 || type==0x8035 || type==0x86dd
	if(type>0x05DC)
	{
		debug("Ethernet Type:%04X",type);
		snprintf(db_ip->eth_type,sizeof(db_ip->eth_type),"%04X",type);
		pcap_ethernet_rfc894(type);
	}else
	{
		debug("Ethernet Length:%04X",type);
		snprintf(db_ip->eth_length,sizeof(db_ip->eth_length),"%d",type);
		pcap_ethernet_8023(packet);
	}
	return 0;
}

int pcap_ipv4(const u_char *packet,db_ip_t *db_ip)
{
	// The IP header
	const struct packet_ip *ip;
	// IP头
	ip = (struct packet_ip*)(packet + SIZE_ETHERNET);

	// The IP header Length
	u_short ip_h_len;
	u_short ip_len;

	ip_h_len = IP_HL(ip)*4;
	if (ip_h_len < 20) {
		error("无效的IP头长度: %u bytes",ip_h_len);
		return -1;
	}
	ip_len=HL_LH(ip->ip_len);

	//ntohs(ip->ip_len)
	debug("fpcap_name:%s fpcap_id:%s", db_ip->fpcap_name, db_ip->fpcap_id);
	debug("IP header len:%lu ip_len:%lu ip_len:%04X",ip_h_len,ip_len,ip_len);
	debug("IP protocol:%d IPPROTO_TCP:%04X IPPROTO_UDP:%04X IPPROTO_ICMP:%04X",ip->ip_p,IPPROTO_TCP,IPPROTO_UDP,IPPROTO_ICMP);

	snprintf(db_ip->ip_length,sizeof(db_ip->ip_length),"%d",ip_len);

	char src_addr[16];
	char dst_addr[16];
	char * ipaddr = NULL;

	ipaddr = inet_ntoa(ip->ip_src);
	strcpy(src_addr,ipaddr);

	snprintf(db_ip->ip_src,sizeof(db_ip->ip_src),"%s",src_addr);

	ipaddr = inet_ntoa(ip->ip_dst);
	strcpy(dst_addr,ipaddr);

	snprintf(db_ip->ip_dst,sizeof(db_ip->ip_dst),"%s",dst_addr);

	debug("IP src:%s\tdst:%s",src_addr,dst_addr);

	//TCP,UDP,ICMP
	switch(ip->ip_p)
	{
		case IPPROTO_TCP:
			snprintf(db_ip->ip_protocol,sizeof(db_ip->ip_protocol),"%s","tcp");
			pcap_tcp(packet,db_ip,ip_h_len,ip_len);
			break;
		case IPPROTO_UDP:
			snprintf(db_ip->ip_protocol,sizeof(db_ip->ip_protocol),"%s","udp");
			pcap_udp(packet,db_ip,ip_h_len,ip_len);
			break;
		case IPPROTO_ICMP:
			snprintf(db_ip->ip_protocol,sizeof(db_ip->ip_protocol),"%s","icmp");
			break;
		default:
			break;
	}
	return 0;
}

void pcap_ipv6(const u_char *packet,db_ip_t *db_ip)
{
	debug("IPV6");
}

void pcap_ipllc(const u_char *packet,db_ip_t *db_ip)
{
	debug("IP LLC");
}

int pcap_ip(const struct pcap_pkthdr *header,const u_char *packet,db_ip_t *db_ip)
{
	pcap_frame(header,db_ip);

	pcap_ethernet(packet,db_ip);

	// The IP header
	const struct packet_ip *ip;
	// IP头
	ip = (struct packet_ip*)(packet + SIZE_ETHERNET);

	snprintf(db_ip->ip_vhl,sizeof(db_ip->ip_vhl),"%02x",ip->ip_vhl);
	snprintf(db_ip->ip_tos,sizeof(db_ip->ip_tos),"%02x",ip->ip_tos);

	debug("IP ip->ip_vhl %02x",(ip->ip_vhl & 0xf0));
	//IP Version
	switch(ip->ip_vhl & 0xf0)
	{
		case 0x40://IPV4
			pcap_ipv4(packet,db_ip);
			break;
		case 0x60://IPV6
			pcap_ipv6(packet,db_ip);
			return -6;
		case 0xa0://IP LLC
			pcap_ipllc(packet,db_ip);
			return -10;
		default:
			return -11;
	}
	return 0;
}

int pcap_tcp(const u_char *packet,db_ip_t *db_ip,const u_int ip_h_len,const u_int ip_len)
{
	// The TCP header
	const struct packet_tcp *tcp;
	// Packet payload
	char *payload;

	//TCP Header length
	int tcp_h_len;

	//TCP Payload length
	int tcp_payload_len;

	// TCP头 
	tcp = (struct packet_tcp*)(packet + SIZE_ETHERNET + ip_h_len);
	tcp_h_len = TH_OFF(tcp)*4;
	if (tcp_h_len < 20) {
		//printf("无效的TCP头长度: %u bytes\n", size_tcp);
		return -1;
	}

	int sport =  ntohs(tcp->th_sport);
	int dport =  ntohs(tcp->th_dport);
//inet_ntoa(ip->ip_src), 
//inet_ntoa(ip->ip_dst),
	snprintf(db_ip->ip_src_port,sizeof(db_ip->ip_src_port),"%d",sport);
	snprintf(db_ip->ip_dst_port,sizeof(db_ip->ip_dst_port),"%d",dport);

	debug("TCP %d -> %d",sport,dport);

	//内容
	payload = (char *)(packet + SIZE_ETHERNET + ip_h_len + tcp_h_len);
	//内容长度
	tcp_payload_len = ip_len - (ip_h_len + tcp_h_len);

	if(tcp_payload_len > 0){
		//侦听模块
		review_tcp(payload,db_ip);
	}
	return 0;
}

int pcap_udp(const u_char *packet,db_ip_t *db_ip,const u_int ip_h_len,const u_int ip_len)
{
	// The UDP header
	const struct packet_udp *udp;
	// Packet payload
	char *payload;

	//UDP Payload length
	int udp_payload_len;

	// UDP 头 
	udp = (struct packet_udp*)(packet + SIZE_ETHERNET + ip_h_len);

	int sport =  ntohs(udp->th_sport);
	int dport =  ntohs(udp->th_dport);
	snprintf(db_ip->ip_src_port,sizeof(db_ip->ip_src_port),"%d",sport);
	snprintf(db_ip->ip_dst_port,sizeof(db_ip->ip_dst_port),"%d",dport);

	debug("UDP port %d -> %d",sport,dport);

	//内容
	payload = (char *)(packet + SIZE_ETHERNET + ip_h_len + 8);
	//内容长度
	udp_payload_len = ntohs(udp->th_len);

	if(udp_payload_len > 0){
		//侦听模块
		review_udp(payload,db_ip);
	}
	return 0;
}

//pcap解析 基于libpcap
int pcap_analysis(const struct pcap_pkthdr *header, const u_char *packet)
{
	//捕获数据包 自己解析
	//const struct packet_ethernet *ethernet;	// The ethernet header [1]
	const struct packet_ip *ip;				// The IP header
	const struct packet_tcp *tcp;			// The TCP header
	//char *payload;					// Packet payload

	int size_ip;
	int size_tcp;
	int size_payload;

	// 以太网头
	//ethernet = (struct packet_ethernet*)(packet);

	// IP头 
	ip = (struct packet_ip*)(packet + SIZE_ETHERNET);
	size_ip = IP_HL(ip)*4;
	if (size_ip < 20) {
		//printf("无效的IP头长度: %u bytes\n", size_ip);
		return -1;
	}

	if ( ip->ip_p != IPPROTO_TCP ){ // TCP,UDP,ICMP,IP
	return -2;
	}

	// TCP头 
	tcp = (struct packet_tcp*)(packet + SIZE_ETHERNET + size_ip);
	size_tcp = TH_OFF(tcp)*4;
	if (size_tcp < 20) {
		//printf("无效的TCP头长度: %u bytes\n", size_tcp);
		return -3;
	}

	//int sport =  ntohs(tcp->th_sport);
	//int dport =  ntohs(tcp->th_dport);
	//printf("%s:%d -> ", inet_ntoa(ip->ip_src), sport);
	//printf("%s:%d ", inet_ntoa(ip->ip_dst), dport);

	//内容
	//payload = (char *)(packet + SIZE_ETHERNET + size_ip + size_tcp);

	//内容长度 
	size_payload = ntohs(ip->ip_len) - (size_ip + size_tcp);

	if (size_payload > 0) {
		//printf("%d bytes:\n", size_payload, payload);
		printf("seq:%d ack:%d flags:%d bytes:%d\n",ntohs(tcp->th_seq),ntohs(tcp->th_ack),ntohs(tcp->th_flags),size_payload);
	} else {
		printf("seq:%d ack:%d syn:%d payload zero\n", ntohs(tcp->th_seq), ntohs(tcp->th_ack), ntohs(TH_SYN));
	}

	return 0;
}
