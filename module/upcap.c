#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include <pcap.h>

#include "ulog.h"
#include "linklist.h"
#include "thpool.h"

#include "ufile.h"
#include "ethernet.h"
#include "db_ethernet.h"
#include "db_service.h"
#include "ulibpcap/cf_pcap.h"
#include "uboot.h"

extern boot_parameters_t para;

void usage(char *p)
{
	fprintf(stdout, "%s -branch [1] -dev [enp0s31f6] -log [/var/tmp/ulibpcap.log] -save [/var/tmp]\n",p);
	fprintf(stdout, "%s -branch [1] -dev enp0s31f6 -log /var/tmp/ulibpcap.log -save /var/tmp\n",p);
	fprintf(stdout, "%s -branch 1 -log /tmp/ulibpcap.log -rpcap[read_pcap] /var/tmp\n",p);
}

int main(int argc,char **argv,char **envp)
{
	char *dev = NULL;

	if (argc < 2)
	{
		usage(*argv);
		exit(1);
	}

	initialize(NULL,NULL,cf_boot_read);

	int k = 0;
	while (argc > k) {
		//printf("%s\n",argv[k]);
		if ((strcmp(argv[k],"?") == 0) || (strcmp(argv[k],"--help") == 0)){
			usage(*(++argv));
			return 1;
			//c = atoi(argv[k]);
		}

		if (strcmp(argv[k],"-branch") == 0){
			para.branch_id = atoi(*(argv + (++k)));
			//printf("b1:%s\n",*(argv + (++k)));
			//printf("b2:%lu\n",branch_id);
		}

		if (strcmp(argv[k],"-dev") == 0){
			//dev = argv[++k];
			dev = *(argv + (++k));
		}

		if (strcmp(argv[k],"-log") == 0){
			logInit(argv[++k],0);
			//logsk(1024*1024);
		}

		if ((strcmp(argv[k],"-save") == 0)){
			snprintf(para.save_pcap,sizeof(para.save_pcap),"%s",argv[++k]);
			//创建目录
			mkdir_p(para.save_pcap,strlen(para.save_pcap));
		}
		
		if ((strcmp(argv[k],"-rpcap") == 0)){
			snprintf(para.read_pcap,sizeof(para.read_pcap),"%s",argv[++k]);
			//创建目录
			//mkdir_p(pcap_path,strlen(pcap_path));
			ufile_list(para.read_pcap,para.branch_id);
			goto end;
		}
		k++;
	}
	printf("\n");

	service_start(dev);
	
	end:
	release();

	return(0);
}
