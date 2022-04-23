#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <pcap.h>

#include <unistd.h>

#include "ulog.h"

#ifdef DUMPPCAP_MAIN
#include "ulibpcap/dumppcap.h"
#endif

#ifdef CF_PCAP_MAIN
#include "ulibpcap/cf_pcap.h"
#endif

#if (defined FSERVER_MAIN) || (defined FCLIENT_MAIN)
#include <stdlib.h>
#include "extra/slave.h"
#endif

#ifdef DUMPPCAP_MAIN
//启动参数
typedef struct _boot_parameters{
	//分部ID
	u_short branch_id;
	//pcap文件路径
	char read_pcap[128];

	char *dev;
	char *cf_name;

	char save_pcap[128];

	char filter_rule[256];

	char status;

	char lock_file[128];
	
	int stop;
}boot_parameters_t;

boot_parameters_t para;

void usage(char *p)
{
	fprintf(stdout, "%s -i[enp0s31f6] -r -h -v\n",p);
}

extern char *optarg;
extern int optind, opterr, optopt;

int cli_main(int argc,char **argv,char **envp)
{
	opterr=0;

	//长选项结构体数组
	static struct option long_opts[] = {
		{"help",	no_argument,			NULL,	'h'},
		{"version",	no_argument,			NULL,	'v'},
		{"dev",		optional_argument,		0,		'i'},
		{"read file",	required_argument,	0,		'r'},
		{0,			0,					0,		0}
	};

	//argc argv ：直接从main函数传递而来
	//short_opts：短选项字符串。如”n:v"，这里需要指出的是，短选项字符串不需要‘-’，而且但选项需要传递参数时，在短选项后面加上“：”。
	//long_opts/long_options：struct option 数组，用于存放长选项参数。
	//long_ind：用于返回长选项在long_opts结构体数组中的索引值，用于调试。一般置为NULL

	int required=0;

	//用于接收字符选项
	int opt;

	char *short_opts = "h::v::i:r:";
	int long_ind = 0;

	//opt==-1 参数结束
	while((opt = getopt_long(argc, argv, short_opts, long_opts, &long_ind))!= -1)
	{
		//printf("opt=%c\toptarg=%s\toptind=%d\t*argv=%s\n", opt,optarg,optind,*(argv+optind));
		switch (opt) {	//获取参数解析
			case 0:
				break;
			case 'i':
				para.dev = optarg;
				break;
			case 'r':
				break;
			case 'h':
				usage(*argv);
				return 1;
			case 'v':
				printf("ver 1.0\n");
				return 1;
			case '?':
			default:
				break;
		}
	}

	//printf("opt=%d\toptarg=%s\toptind=%d\t*argv=%s\n", opt,optarg,optind,*(argv+optind));
	//printf("optind=%d\topterr=%d\toptopt=%d\toptarg=%s\targc=%d\n",optind,opterr,optopt,optarg,argc);
	/*printf("optind=%d\topterr=%d\toptopt=%d\toptarg=%s\targc=%d\n",optind,opterr,optopt,optarg,argc);
	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", *(argv+optind++));
		printf("\n");
	}*/

	required = (para.dev!=NULL || para.cf_name!=NULL)?1:0;
	if(!required)
	{
		usage(*argv);
		return 1;
	}

	debug("cli_main promiscuous");

	promiscuous(para.dev,NULL,NULL);

	return 0;
}
#endif

#ifdef OPT_MAIN

extern char *optarg;
extern int optind, opterr, optopt;

int get_opts(int argc,char **argv,char **envp)
{
	opterr=0;

	//长选项结构体数组
	static struct option long_opts[] = {
		{"help",		no_argument,			NULL,	'h'},
		{"version",		no_argument,			NULL,	'v'},
		{"dev",			optional_argument,		0,		'i'},
		{"read file",	required_argument,		0,		'r'},
		{0,				0,						0,		0}
	};

	//用于接收字符选项
	int opt;
	char *short_opts = "h::v::i:r:";
	int long_ind = 0;

	int required=0;
	//opt==-1 参数结束
	while((opt = getopt_long(argc, argv, short_opts, long_opts, &long_ind))!= -1)
	{
		//printf("opt=%c\toptarg=%s\toptind=%d\t*argv=%s\n", opt,optarg,optind,*(argv+optind));
		switch (opt) {	//获取参数解析
			case 0:
				break;
			case 'i':
				para.dev = optarg;
				break;
			case 'r':
				break;
			case 'h':
				usage(*argv);
				return 1;
			case 'v':
				printf("ver 1.0\n");
				return 1;
			case '?':
			default:
				break;
		}
	}

	required = (para.dev!=NULL || para.cf_name!=NULL)?1:0;
	if(!required)
	{
		usage(*argv);
		return 1;
	}

	return 0;
}
#endif

#ifdef CF_PCAP_MAIN
extern int cf_pcap_save(const char *save_pcap,const char *hexs,size_t size);
#endif

#ifdef DUMP_BINARY_MAIN
extern void epan_cf(const char *cf_name);
#endif

#if (defined CF_PCAP_MAIN) && (defined DUMP_BINARY_MAIN)
void make_pcap(const char *hexs,size_t size)
{
	const char *cf_name="/tmp/dumpbinary.pcap";
	if(access(cf_name,F_OK|R_OK)!=-1)
	{
		remove(cf_name);
	}
	cf_pcap_save(cf_name,hexs,strlen(hexs));
	debug("tmpfile:%s",cf_name);
	epan_cf(cf_name);
}
#endif

int main(int argc,char **argv,char **envp)
{
	log_set_level(1);

	#ifdef CF_PCAP_MAIN
	const char *hexs="081079a49f7854e1ad1b1f92080045000028a4bc4000400624b5ac19065a3d81816a99680050658662db7af814915010ffff71790000";
	const char *buf_file="/tmp/dumpbinary.pcap";
	#endif

	#if (defined CF_PCAP_MAIN) && (defined DUMP_BINARY_MAIN)
	log_set_level(0);
	debug("make_pcap");
	const char *hex_payload=hexs;
	hex_payload=(const char *)*(argv+1);
	make_pcap(hex_payload,strlen(hex_payload));
	return 0;
	#endif

	#ifdef DUMPPCAP_MAIN
	debug("cli_main");
	cli_main(argc,argv,envp);
	return 0;
	#endif

	#ifdef CF_PCAP_MAIN
	debug("cf_pcap_save");
	cf_pcap_save(buf_file,hexs,strlen(hexs));
	return 0;
	#endif

	#ifdef DUMP_BINARY_MAIN
	debug("epan_cf");
	epan_cf(*(argv+1));
	//epan_packet();
	return 0;
	#endif

	#ifdef FSERVER_MAIN
	debug("example_fserver");
	fserver(SLAVE_PORT);
	return 0;
	#endif

	#ifdef FCLIENT_MAIN
	if(argc<4)
	{
		printf("%s [ip] [port] [file_path]\n",argv[0]);
		return 1;
	}
	debug("example_fclient");
	fclient(argv[1],atoi(argv[2]),argv[3],strlen(argv[3]));
	return 0;
	#endif

	return(0);
}
