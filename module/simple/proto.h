#ifndef F_PCAP_H
#define F_PCAP_H

//定义错误码
#define ERROR_FILE_OPEN_FAILED -1
#define ERROR_MEM_ALLOC_FAILED -2
#define ERROR_PCAP_PARSE_FAILED -3

#define MAX_ETH_FRAME 1514

//定义以太帧
#define SNAP_LEN 1518		//以太网帧最大长度
#define SIZE_ETHERNET 14	//以太网包头长度 mac 6*2, type: 2
#define ETHER_ADDR_LEN 6	//mac地址长度
//#define ETHER_MAX_LEN 1500		//以太网帧最大长度

#define IP_HL(ip)			(((ip)->ip_vhl) & 0x0f)
#define IP_V(ip)			(((ip)->ip_vhl) >> 4)

#define HL_LH(ushort_x) ( (ushort_x & 0xff00) >> 8) | ( (ushort_x & 0x00ff ) << 8)

//Ethernet header
struct packet_ethernet {
	u_char	ether_mac_dst[ETHER_ADDR_LEN];	// destination host address
	u_char	ether_mac_src[ETHER_ADDR_LEN];	// source host address
	u_short	ether_type;					 	// IP? ARP? RARP? etc
	u_char dsap;
	u_char ssap;
	u_char cntl;
	u_char	org_code[3];
	u_short	ether_type_1042;
};

//IP header
struct packet_ip {
	u_char  ip_vhl;				// version << 4 | header length >> 2
	u_char  ip_tos;				// type of service
	u_short ip_len;				// total length
	u_short ip_id;				// identification
	u_short ip_off;				// fragment offset field
	#define IP_RF 0x8000		// reserved fragment flag
	#define IP_DF 0x4000		// dont fragment flag
	#define IP_MF 0x2000		// more fragments flag
	#define IP_OFFMASK 0x1fff	// mask for fragmenting bits
	u_char  ip_ttl;				// time to live
	u_char  ip_p;				// protocol
	u_short ip_sum;				// checksum 
	struct in_addr ip_src;		// source and dest address
	struct in_addr ip_dst;		// source and dest address
};

// TCP header
typedef u_int tcp_seq;
struct packet_tcp {
	u_short th_sport;			//source port
	u_short th_dport;			// destination port 
	tcp_seq th_seq;				// sequence number
	tcp_seq th_ack;				// acknowledgement number
	u_char  th_offx2;			// data offset, rsvd 
	#define TH_OFF(th)	(((th)->th_offx2 & 0xf0) >> 4)
	u_char  th_flags;
	#define TH_FIN  0x01
	#define TH_SYN  0x02
	#define TH_RST  0x04
	#define TH_PUSH 0x08
	#define TH_ACK  0x10
	#define TH_URG  0x20
	#define TH_ECE  0x40
	#define TH_CWR  0x80
	#define TH_FLAGS		(TH_FIN|TH_SYN|TH_RST|TH_ACK|TH_URG|TH_ECE|TH_CWR)
	u_short th_win;				// window 
	u_short th_sum;				// checksum 
	u_short th_urp;				// urgent pointer
};

// UDP header
struct packet_udp {
	u_short th_sport;			//source port
	u_short th_dport;			// destination port
	u_char  th_len;
	u_short th_sum;				// checksum 
};


int pcap_analysis(const struct pcap_pkthdr *header, const u_char *packet);

int pcap_ethernet(const u_char *packet,db_ip_t *db_ip);
int pcap_ip(const struct pcap_pkthdr *header,const u_char *packet,db_ip_t *db_ip);

int pcap_tcp(const u_char *packet,db_ip_t *db_ip,const u_int ip_h_len,const u_int ip_len);
int pcap_udp(const u_char *packet,db_ip_t *db_ip,const u_int ip_h_len,const u_int ip_len);

#endif
