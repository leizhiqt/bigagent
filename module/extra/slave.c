#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "ulog.h"
#include "linklist.h"
#include "ustring.h"
#include "slave.h"

/*
tcp 客户端上		传文件
*/
int fclient(const char *ip_addr,const int port,const char *file_name,const int file_name_len)
{
	int   sockfd, len;
	char  buffer[BUFF_LIEN_SIZE];
	struct sockaddr_in  servaddr;
	FILE *fq;
debug("ip_addr %s port %d file %s",ip_addr,port,file_name);
	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		debug("create socket error: %s(errno: %d)", strerror(errno),errno);
		return 0;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);

	if(inet_pton(AF_INET, ip_addr, &servaddr.sin_addr) <= 0){
		debug("inet_pton error for %s",ip_addr);
		return 0;
	}

	if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
		debug("connect error: %s(errno: %d)",strerror(errno),errno);
		return 0;
	}

	//write(sockfd,"fname:",6);
	//file_nmae
	write(sockfd,file_name,file_name_len);
	write(sockfd,"\0",1);

	if((fq = fopen(file_name,"rb")) == NULL ){
		debug("File open.");
		close(sockfd);
		exit(1);
	}

	bzero(buffer,sizeof(buffer));
	while(!feof(fq)){
		len = fread(buffer, 1, sizeof(buffer),fq);
		if(len != write(sockfd, buffer, len)){
			debug("write.");
			break;
		}
	}

	close(sockfd);
	fclose(fq);

	return 0;
}

/*
tcp 服务端接		收文件
*/
int fserver(const int port)
{
	int		listenfd,connfd;
	struct sockaddr_in servaddr;
	char	buff[BUFF_LIEN_SIZE];
	FILE	*fp;
	int		n;

	if( (listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1 )
	{
		debug("create socket error: %s(errno: %d)",strerror(errno),errno);
		return 0;
	}
	debug("init fserver");

	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	//设置端口可重用
	int contain;
	setsockopt(listenfd,SOL_SOCKET, SO_REUSEADDR, &contain, sizeof(int));

	if( bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1){
		debug("bind socket error: %s(errno: %d)",strerror(errno),errno);
		return 0;
	}
	debug("bind sucess");

	if( listen(listenfd, 10) == -1)
	{
		debug("listen socket error: %s(errno: %d)",strerror(errno),errno);
		return 0;
	}

	debug("waiting for client's request");

	struct sockaddr_in client_addr;
	socklen_t size=sizeof(client_addr);

	while(1)
	{

		if( (connfd = accept(listenfd, (struct sockaddr*)&client_addr, &size)) == -1)
		{
			debug("accept socket error: %s(errno: %d)",strerror(errno),errno);
			continue;
		}

		memset(buff,'\0',BUFF_LIEN_SIZE);
		n=read(connfd, buff, BUFF_LIEN_SIZE);

		//debug("req up file:%s %d %d",buff,n,strlen(buff));

		debug("req %s ",buff);

		char save_fname[200];
		int lastn = lastIndexOf(buff,"/");
		substring(save_fname,buff,lastn+1,strlen(buff));

		char save_full[200];
		memset(save_full,'\0',sizeof(save_full));

		const char *save_path="/opt/agent/uppcap";
		strcat(save_full,save_path);
		strcat(save_full,"/");
		strcat(save_full,save_fname);

		//获取文件名称
		if((fp = fopen(save_full,"wb+") ) == NULL )
		{
			debug("File");
			close(listenfd);
			continue;
		}
		debug("save to %s",save_full);

		while((n=read(connfd, buff, BUFF_LIEN_SIZE))){
			//debug("read bytes: %d", n);
			fwrite(buff, 1, n, fp);
		}

		buff[n] = '\0';
		//debug("recv msg from client: %s", buff);
		fclose(fp);

		close(connfd);
		debug("listen client again");
	}

	close(listenfd);
	return 0;
}

void *fserver_pthread(void *v)
{
	const int *port = (int *)v;
	fserver(*port);
	return NULL;
}
