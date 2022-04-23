#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <sys/types.h>
#include <sys/stat.h>

//#include "times.h"
#include "ulog.h"

static int nk = 1024;

//0 close
//1 debug 2 error
static int log_level = 1;

static char logfile[64]={0};

void log_set_level(int level)
{
	log_level = level;
}

int log_get_level()
{
	return log_level;
}

//0 close
//1 debug 2 error
void logInit(const char *s,int level)
{
	memset(logfile,'\0',sizeof(logfile));
	snprintf(logfile,sizeof(logfile),"%s",s);

	log_set_level(level);
}

void logsk(const int sk)
{
	nk = sk;
}

unsigned long sizeof_f(const char *path)
{
	unsigned long filesize = -1;
	struct stat statbuff;

	if(stat(path, &statbuff) < 0)
	{
		return filesize;
	}
	return statbuff.st_size;
}

int log_save(const char *file_name,const char *buf,int mod)
{
	int ret = -1;
	int fd = -1;

	if((fd = open(file_name,mod,0644))==-1)
	{
		fprintf(stderr," open:%s RET:%d\n",file_name,ret);
		return -1;
	}

	if((ret = write(fd, buf, strlen(buf)))==-1)
	{
		fprintf(stderr," write:%s RET:%d\n",file_name,ret);
		return -2;
	}

	//release:
	if((ret = close(fd))==-1)
	{
		fprintf(stderr," close:%s RET:%d\n",file_name,ret);
		return -3;
	}

	return 0;
}

int log_append(const char *file_name,const char *buf)
{
	return log_save(file_name,buf,O_WRONLY|O_CREAT|O_APPEND);
}

int log_flush(const char *file_name,const char *buf)
{
	return log_save(file_name,buf,O_WRONLY|O_CREAT|O_TRUNC);
}

void printfs(FILE *stream,const char *fmt, ...){
	char msg[LOG_LINE_SIZE+22]	={0};
	char buf[LOG_LINE_SIZE]		={0};
	char times[20]			={0};

	va_list va_alist;
	va_start(va_alist, fmt);
	vsnprintf(buf, sizeof(buf), fmt, va_alist);
	va_end (va_alist);

	t_stime(times);
	snprintf(msg,sizeof(msg),"%s %s\n",times,buf);

	if(log_level || stream==stderr) fprintf(stream,"%s",msg);

	if(strlen(logfile)<1 || !log_level)
	{
		//printf("%s is NULL\n",logfile);
		return;
	}

	//printf("logfile:%s->{%s}\n",logfile,msg);

	unsigned long fsize = sizeof_f(logfile);
	int k=1024;

	if(fsize>nk*k)
	{
		log_flush(logfile,msg);
		return;
	}

	log_append(logfile,msg);
}

void printbytes(const char * bytes,const int n)
{
	for(int i=0;i<n;i++)
	{
		printf("bytes[%d]:%02X\n",i,bytes[i]);
	}
}
