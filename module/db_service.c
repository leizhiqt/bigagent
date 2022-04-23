#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "linklist.h"
#include "ustring.h"

#include "ulog.h"
#include "ethernet.h"
#include "db_ethernet.h"
#include "db_service.h"

//16进制保存到mysql数据库
void db_save_payload(const char *payload,const uint payload_len,db_ip_t *db_ip)
{
	char *binary=(char *)payload;

	int hexs_len=2*payload_len*sizeof(char)+1;
	char *hexs=NULL;

	hexs = (char *) malloc(hexs_len);
	memset(hexs,'\0',hexs_len);

	binary_to_hex(hexs,hexs_len-1,binary,payload_len);

	db_ip->payload_len=hexs_len-1;
	db_ip->payload=hexs;

	db_ethernet_add(db_ip);

	free(hexs);
}
