#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#include "ulog.h"
#include "ethernet.h"
#include "forward.h"
#include "forward_udp.h"

int forward_udp_send(const addr_node_t **forward_pool,const int forward_pool_size,const char *msg,const int len)
{
	for(int i=0;i<forward_pool_size;i++)
	{
		const addr_node_t *addr_node = *(forward_pool+i);

		//debug("send to %d:%s:%d",addr_node->sockfd,addr_node->ip_addr,addr_node->port);
		if(!(addr_node->sockfd>0)) return -1;

		int n=sendto(addr_node->sockfd,msg,len,0,(struct sockaddr*)&addr_node->addr,sizeof(addr_node->addr));
		if(n<0)
		{
			error("send error %d",n);
			return 0;
		}
		//debug("send %d Success %d",addr_node->sockfd,n);
	}
	return -2;
}
