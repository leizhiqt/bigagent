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

#include "ulog.h"
#include "usys.h"

/*
	列表设备数目
*/
int cdev(const char * cmd)
{
	int k = 0;

	FILE *fstream=NULL;	
	char buff[1024];  
	memset(buff,0,sizeof(buff));

	if(NULL==(fstream=popen("ls -l /dev/video*","r")))	
	{   
		//fprintf(stderr,"execute command failed: %s",strerror(errno));
		return -1;	
	}   

	while(NULL!=fgets(buff, sizeof(buff), fstream))   
	{   
		printf("%s",buff);
		k++;
	} 

	pclose(fstream);  
	return k;
}

/*
	sendmail发送邮件
	const char *buf		邮件内容
	const char *subject	邮件标题
	const char *attach		附件全路径
	const char *to_addr	收件人邮箱地址
*/
void send_email(const char *buf,const char *subject,const char *attach,const char *to_addr)
{
	//echo "this is my test mail" | mail  -s 'mail test' -a a.txt   1920849305@qq.com
	////sendmail 发送格式
	char send_email[1024];
	snprintf(send_email,sizeof(send_email),"echo \"%s\" | mail  -s '%s' -a %s %s",buf,subject,attach,to_addr);
	debug("send_email:%s",send_email);
	system(send_email);
}
