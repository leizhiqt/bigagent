//socket udp 客户端
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main()
{
	//创建socket对象
	int sockfd=socket(AF_INET,SOCK_DGRAM,0);

	//创建网络通信对象
	struct sockaddr_in addr;
	addr.sin_family =AF_INET;
	addr.sin_port =htons(1234);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	//while(1)
	//{
		//printf("请输入一个数字：");
		//sendto
		//char buf=0;
		//scanf("%hhd",&buf);
		char *msg = "alert ftp and 4389 error\n";
		
		int n=sendto(sockfd,msg,strlen(msg),0,(struct sockaddr*)&addr,sizeof(addr));
		if(n<0) printf("send error\n");
		printf("send Success %d\n",n);

		//socklen_t len=sizeof(addr);
		//recvfrom(sockfd,&buf,sizeof(buf),0,(struct sockaddr*)&addr,&len);

		/*if(66 ==buf)
		{
			printf(" server 成功接受\n");
		}
		else
		{
			printf("server 数据丢失\n");
		}*/

	//}
	close(sockfd);
}
