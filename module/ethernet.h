#ifndef ETHERNET_H
#define ETHERNET_H

struct _db_ip{
	u_short branch_id;
	u_char fpcap_name[256];
	u_char fpcap_id[64];
	u_char frame_tv[64];
	u_char frame_caplen[5];
	u_char frame_len[5];
	u_char eth_src[18];
	u_char eth_dst[18];
	u_char eth_type[17];
	u_char eth_length[17];
	u_char ip_vhl[5];
	u_char ip_tos[5];
	u_char ip_src[16];
	u_char ip_dst[16];
	u_char ip_length[17];
	u_char ip_protocol[128];
	u_char ip_src_port[6];
	u_char ip_dst_port[6];
	u_char alert_msg[65];
	const char *payload;
	u_int payload_len;
	time_t t_secs;//长整形时间
};
typedef struct _db_ip db_ethernet_t;
typedef struct _db_ip db_ip_t;

void db_ip_init(db_ip_t *db_ip);
void db_ip_print(db_ip_t *db_ip);
#endif
