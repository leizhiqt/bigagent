#ifndef _USYS_H_
#define _USYS_H_

/*
	列表设备数目
*/
int cdev(const char * cmd);

/*
	sendmail发送邮件
	const char *buf			邮件内容
	const char *subject		邮件标题
	const char *attach		附件全路径
	const char *to_addr		收件人邮箱地址
*/
void send_email(const char *buf,const char *subject,const char *attach,const char *to_addr);

#endif
