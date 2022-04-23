#include <stdio.h>
#include <string.h>

#include <pcap.h>

#include "ulog.h"
#include "dtype.h"
#include "ethernet.h"

void db_ip_init(db_ip_t *db_ip)
{
	db_ip->branch_id=0;
	memset(db_ip->fpcap_name,'\0', sizeof(db_ip->fpcap_name));
	memset(db_ip->fpcap_id,'\0', sizeof(db_ip->fpcap_id));
	
	memset(db_ip->frame_tv,'\0', sizeof(db_ip->frame_tv));
	memset(db_ip->frame_caplen,'\0', sizeof(db_ip->frame_caplen));
	memset(db_ip->frame_len,'\0', sizeof(db_ip->frame_len));
	
	memset(db_ip->eth_src,'\0', sizeof(db_ip->eth_src));
	memset(db_ip->eth_dst,'\0', sizeof(db_ip->eth_dst));
	memset(db_ip->eth_type,'\0', sizeof(db_ip->eth_type));
	memset(db_ip->eth_length,'\0', sizeof(db_ip->eth_length));
	
	memset(db_ip->ip_vhl,'\0', sizeof(db_ip->ip_vhl));
	memset(db_ip->ip_tos,'\0', sizeof(db_ip->ip_tos));
	memset(db_ip->ip_src,'\0', sizeof(db_ip->ip_src));
	memset(db_ip->ip_dst,'\0', sizeof(db_ip->ip_dst));
	memset(db_ip->ip_length,'\0', sizeof(db_ip->ip_length));
	memset(db_ip->ip_protocol,'\0', sizeof(db_ip->ip_protocol));
	memset(db_ip->ip_src_port,'\0', sizeof(db_ip->ip_src_port));
	memset(db_ip->ip_dst_port,'\0', sizeof(db_ip->ip_dst_port));

	memset(db_ip->alert_msg,'\0', sizeof(db_ip->alert_msg));

	db_ip->payload=NULL;
	db_ip->payload_len=0;
	db_ip->t_secs=0;
}

void db_ip_print(db_ip_t *db_ip)
{
	debug("branch_id=%d",db_ip->branch_id);
	debug("fpcap_name=%s",db_ip->fpcap_name);
	debug("fpcap_id=%s",db_ip->fpcap_id);
	debug("frame_tv=%s",db_ip->frame_tv);
	debug("frame_len=%s",db_ip->frame_len);
	debug("eth_src=%s",db_ip->eth_src);
	debug("eth_dst=%s",db_ip->eth_dst);
	debug("eth_length=%s",db_ip->eth_length);
	debug("ip_vhl=%s",db_ip->ip_vhl);
	debug("ip_tos=%s",db_ip->ip_tos);
	debug("ip_src=%s",db_ip->ip_src);
	debug("ip_dst=%s",db_ip->ip_dst);
	debug("ip_length=%s",db_ip->ip_length);
	debug("ip_protocol=%s",db_ip->ip_protocol);
	debug("ip_src_port=%s",db_ip->ip_src_port);
	debug("ip_dst_port=%s",db_ip->ip_dst_port);
	debug("payload=%s",db_ip->payload);
	debug("payload_len=%d",db_ip->payload_len);
	debug("t_secs=%d",db_ip->t_secs);
}
