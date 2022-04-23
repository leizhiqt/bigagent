#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

//#include <pcap.h>

//#include <glib.h>

//#include "epan/epan.h"
//#include "epan/epan_dissect.h"
//#include "epan/proto.h"
//#include "epan/packet_info.h"
//#include "epan/frame_data.h"
//#include "epan/packet.h"

#include "ulog.h"
#include "ulibpcap/cf_pcap.h"

//#define DATA_LEN 73

// 帧数据,不包括PCAP文件头和帧头
// 数据为ethernet - ipv4 - udp - DNS,上网时随便捕获的.

/*const char data[DATA_LEN] = 
{
0x7E,0x6D,0x20,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x00,0x00,0x08,0x00,0x45,0x00,
0x00,0x3B,0x5F,0x15,0x00,0x00,0x40,0x11,0xF1,0x51,0x73,0xAB,0x4F,0x08,0xDB,0x8D,
0x8C,0x0A,0x9B,0x90,0x00,0x35,0x00,0x27,0xEF,0x4D,0x43,0x07,0x01,0x00,0x00,0x01,
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x74,0x04,0x73,0x69,0x6E,0x61,0x03,0x63,0x6F,
0x6D,0x02,0x63,0x6E,0x00,0x00,0x01,0x00,0x01
};
*/
/*
void print_tree(proto_tree *tree,int level)
{
	if(tree == NULL)
		return;
 
	for(int i=0; i<level; ++i)
		printf("	");
 
	char field_str[ITEM_LABEL_LENGTH + 1] = {0};
	if(tree->finfo->rep == NULL)
		ws_proto_item_fill_label(tree->finfo, field_str);
	else
		strcpy_s(field_str, tree->finfo->rep->representation);
 
	if(!PROTO_ITEM_IS_HIDDEN(tree))
		printf("%s\n", field_str);   
 
	print_tree(tree->first_child, level+1);
	print_tree(tree->next, level);
}
*/
void epan_packet()
{
	
/*
	epan_t	*epan;
	debug("epan_packet");
	epan_dissect_t *edt;
	int file_type_subtype;
	tvbuff_t *tvb;

	column_info *cinfo;
	gint64		data_offset;

	wtap_rec rec;
	Buffer buf;

	edt = epan_dissect_new(epan,TRUE,TRUE);

	wtap_rec_init(&rec);
	ws_buffer_init(&buf,1514);

	frame_data		fdata;
	guint32 cum_bytes;

	frame_data_init(&fdata,0,&rec,data_offset,cum_bytes);

	frame_data		*fd;
	struct packet_provider_data provider;

	debug("epan_packet");
	fd=frame_tvbuff_new_buffer(&provider,fdata,buf);
debug("epan_packet");
	epan_dissect_run(edt,file_type_subtype,&rec,tvb,fd,cinfo);
debug("epan_packet");*/
}

void exec_tshark(const char *cf_name)
{
	//execl("/bin/ls","ls","-l",NULL);

	FILE *fstream = NULL;
	int k=1024;
	int buf_size=10*k;

	char buf[buf_size];
	memset(buf, 0, sizeof(buf));

	char rbuf[1024];
	memset(rbuf, 0, sizeof(rbuf));

	char cmd[128];
	debug("cf_name:%s",cf_name);
	snprintf(cmd,sizeof(cmd),"tshark -V -r %s",cf_name);
	debug("cmd:%s",cmd);
	
	if(NULL == (fstream = popen(cmd,"r")))
	{
		fprintf(stderr,"execute command failed: %s",strerror(errno));
		return;
	}

	while(NULL != fgets(rbuf, sizeof(rbuf), fstream)) 
	{
			//printf("%s",buff);
			//snprintf(buf,"%s",buff);
			strcat(buf,rbuf);
	}

	pclose(fstream);

	//debug("%s",buf);
	printf("%s",buf);
}

void make_pcap(const char *hexs,size_t size)
{
	const char *cf_name="/tmp/dumpbinary.pcap";
	if(access(cf_name,F_OK|R_OK)!=-1)
	{
		remove(cf_name);
	}
	cf_pcap_save(cf_name,hexs,strlen(hexs));
	debug("tmpfile:%s",cf_name);
	exec_tshark(cf_name);
	//epan_cf(cf_name);
}

void usage(char *p)
{
	fprintf(stdout, "%s <hexs> [081079a49f7854e1ad1b1f92080045000028a4bc4000400624b5ac19065a3d81816a99680050658662db7af814915010ffff71790000]\n",p);
}

int main(int argc,char** argv,char **envp)
{
	if (argc < 2)
	{
		usage(*argv);
		exit(1);
	}

	log_set_level(0);

	const char *hex_payload=NULL;
	hex_payload=(const char *)*(argv+1);
	make_pcap(hex_payload,strlen(hex_payload));

	return 0;
}
