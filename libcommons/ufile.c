/********************************** 
 * @author	leizhifesker@icloud.com
 * @date	2017/03/07
 * Last update:	2017/03/07
 * License:	LGPL
 * 
 **********************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <dirent.h>

#include "ulog.h"
#include "ufile.h"


void brline(const char *ch,const int k)
{
	char *p = (char *)ch+k-1;

	if(*p=='\r' || *p=='\n')
		*p='\0';

	p--;

	if(*p=='\r' || *p=='\n')
		*p='\0';
}

void clsline(const char *ch)
{
	char *p = (char *)ch;
	while('\0' != *p)
	{
		//printf("%c %02X\n",*p,*p);
		if(*p=='\r') *p='\0';
		p++;
	}
	//printf("c:%c 0x%02X\n",*p,*p);
	*p = '\0';
	//printf("c:%c 0x%02X\n",*p,*p);
}

int exist_line(const char *file_name,const char *buf)
{
	FILE * fstream;
	char * line = NULL;  
	size_t len = 0;  
	ssize_t read;
	
	if((fstream = fopen(file_name, "r"))==NULL)
		return 0;

	int RET = 0;

	while ((read = getline(&line, &len, fstream)) != -1) {
	
		if(!(read>1)) continue;

		//*(line+read-1) = '\0';
		//printf("Retrieved line of length:%zu length:%d len:%d line:%s buf:%s\n",read,read,len,line,buf);

		if(strcmp(line,buf)==0)
		{
			RET = 1;
			break;
		}
	}

	if(line) free(line);
	fclose(fstream);

	return RET;
}

int save(const char *file_name,const char *buf,const int len,int mod)
{
	int ret = -1;

	int fd = -1;

	if((fd = open(file_name,mod,0644))==-1)
	{
		debug(" open:%s RET:%d",file_name,ret);
		return -1;
	}

	if((ret = write(fd,buf,len))==-1)
	{
		debug(" write:%s RET:%d",file_name,ret);
		return -2;
	}

	//release:
	if((ret = close(fd))==-1)
	{
		debug(" close:%s RET:%d",file_name,ret);
		return -3;
	}

	return 0;
}

int save_txt(const char *file_name,const char *buf){

	//int RET = -1;

	if(exist_line(file_name,buf))
	{
		debug(" exist_line:true");
		return -1;
	}
	return save(file_name,buf,strlen(buf),O_WRONLY|O_CREAT|O_APPEND);
}

int save_append(const char *file_name,const char *buf)
{
	return save(file_name,buf,strlen(buf),O_WRONLY|O_CREAT|O_APPEND);
}

int binary_append(const char *file_name,const char *buf,const int len)
{
	return save(file_name,buf,len,O_WRONLY|O_CREAT|O_APPEND);
}

int save_flush(const char *file_name,const char *buf)
{
	return save(file_name,buf,strlen(buf),O_WRONLY|O_CREAT|O_TRUNC);
}

//tar cmd
void tarz(const char *infile, const char *outfile)
{
	char ecmd[512];
	memset(ecmd,'\0',sizeof(ecmd));
	
	snprintf(ecmd,sizeof(ecmd),"tar -zcf %s %s",outfile,infile);
	debug("ecmd:%s",ecmd);
	system(ecmd);
}

int rm_task(const char * tpath)
{
	int k = 0;

	char ecmd[512];
	memset(ecmd,'\0',sizeof(ecmd));
	
	snprintf(ecmd,sizeof(ecmd),"rm -rf %s",tpath);
	debug("ecmd:%s",ecmd);
	system(ecmd);

	return k;
}

//创建路径
int mkdir_p(const char *fullpath,const uint len)
{
	int k=-1;

	 if (access(fullpath,F_OK|R_OK|W_OK|X_OK) > -1)  
	{
		return 1;
	}
	
	uint fp_len=0;
	fp_len = len+64;

	char buf[fp_len+1];
	char cbuf[fp_len+1];

	strncpy(buf,fullpath,fp_len);
	buf[fp_len]='\0';
	
	memset(cbuf,'\0',sizeof(cbuf));

	if(buf[0]=='/') strcat(cbuf,"/");

	char *token=strtok(buf,"/");
	k=0;
	while(token!=NULL){
		strcat(cbuf,token);
		strcat(cbuf,"/");

		//debug(" mkdir_p k:%d cbuf:%s",k,cbuf);
		if (access(cbuf,F_OK|R_OK|W_OK|X_OK) == -1)  
		{
			k=mkdir(cbuf,S_IRWXU|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
		}
		token=strtok(NULL,"/");
		k++;
	}
	return k;
}

/*
*复制函数
*复制文件
*/
int cp_file(char *destination_path,char* source_path)
{  
	char buffer[1024];  
	FILE *in,*out;//定义两个文件流，分别用于文件的读取和写入int len;  
	if((in=fopen(source_path,"r"))==NULL){//打开源文件的文件流  
		debug("源文件打开失败:%s",source_path);
		return -1; 
	}  
	if((out=fopen(destination_path,"w"))==NULL){//打开目标文件的文件流  
		debug("目标文件创建失败！:%s",destination_path); 
		return -2;  
	}  
	int len;//len为fread读到的字节长  
	while((len=fread(buffer,1,1024,in))>0){//从源文件中读取数据并放到缓冲区中，第二个参数1也可以写成sizeof(char)  
		fwrite(buffer,1,len,out);//将缓冲区的数据写到目标文件中  
	}  
	fclose(out);  
	fclose(in);
	
	return 0;
}

/*
*get_file_size 获取文件大小
*const char *path	文件路径
*/
ulong get_file_size(const char *path)
{
	ulong filesize = -1;
	struct stat statbuff;
	if(stat(path, &statbuff) < 0){
		return filesize;
	}

	filesize = statbuff.st_size;

	return filesize;
}

int read_line(const char *fpath,char *buf,const int buf_size)
{
	FILE * fstream;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;

	if((fstream = fopen(fpath, "r"))==NULL)
		return -1;

	while ((read = getline(&line, &len, fstream)) != -1) {
		if(read>0)
		{
			snprintf(buf,buf_size,"%s",line);
			break;
		}
	}

	if(line) free(line);
	fclose(fstream);

	return read;
}

/*
int read_file()
{
	FILE *fp;

	if ((fp = fopen(jfile, "r")) == NULL)
	{
		error("打开目标文件%s失败\n",jfile);
		return -1;//出错退出
	}

	//获取文件长度;
	fseek(fp,0,SEEK_END);
	int fsize=ftell(fp);

	char *p = (char *) malloc(fsize);
	if(p==NULL) return -2;

	char *p1 = p;
	memset(p,'\0',fsize);

	fseek(fp,0,SEEK_SET);
	int n=0;

	int step=1024;
	char tbuf[step+1];
	memset(tbuf,'\0',step);
	//开始复制文件，文件可能很大，缓冲一次装不下，所以使用一个循环进行读写
	while ((n = fread(tbuf, sizeof(char),step,fp)) > 0)
	{
		//tbuf[n]='\0';
		memcpy(p1,tbuf,n);

		p1 += n;
		memset(tbuf,'\0',step);
	}

	//execute_data(p);

	//strn_repc(p,'.', '_',fsize);
	//execute_json(p);

	free(p);

	fclose(fp);

	//if((access(jfile,F_OK))!=-1)
	//{
	//	remove(jfile);
	//}

	return 0;
}

FILE* output_new()
{
	FILE *fp;

	//now_ns(jfile);
	//strcat(jfile,".json");
	
	//debug("jfile:%s",jfile);

	if ((fp = fopen(jfile, "w")) == NULL)
	{
		debug("打开目标文件%s失败\n",jfile);
		return NULL;//出错退出
	}

	print_stream=print_stream_text_stdio_new(fp);
	jdumper = write_json_preamble(fp);

	return fp;
}

void output_close(FILE *fp)
{
	fclose(fp);
}
*/
