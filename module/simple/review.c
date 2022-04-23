#include <stdio.h>
#include <string.h>

#include <netinet/in.h>

#include <pcap.h>

#include "dtype.h"
#include "ulog.h"
#include "linklist.h"
#include "ustring.h"
#include "ethernet.h"
#include "db_ethernet.h"
#include "db_service.h"
#include "review.h"

/*
命令 | 描述

ABOR：中断数据连接程序
ACCT：系统特权帐号
ALLO：为服务器上的文件存储器分配字节
APPE：添加文件到服务器同名文件
CDUP ：改变服务器上的父目录
CWD ：改变服务器上的工作目录
DELE：删除服务器上的指定文件
HELP：返回指定命令信息
LIST：如果是文件名列出文件信息，如果是目录则列出文件列表
MODE：传输模式（S=流模式，B=块模式，C=压缩模式）
MKD：在服务器上建立指定目录
NLST：列出指定目录内容
NOOP：无动作，除了来自服务器上的承认
PASS：系统登录密码
PASV：请求服务器等待数据连接
PORT ：IP 地址和两字节的端口 ID
PWD：显示当前工作目录
QUIT：从 FTP 服务器上退出登录
REIN：重新初始化登录状态连接
REST：由特定偏移量重启文件传递
RETR：从服务器上找回（复制）文件
RMD：在服务器上删除指定目录
RNFR：对旧路径重命名
RNTO：对新路径重命名
SITE：由服务器提供的站点特殊参数
SMNT：挂载指定文件结构
STAT：在当前程序或目录上返回信息
STOR：储存（复制）文件到服务器上
STOU：储存文件到服务器名称上
STRU：数据结构（F=文件，R=记录，P=页面）
SYST：返回服务器使用的操作系统
TYPE：数据类型（A=ASCII，E=EBCDIC，I=binary）
USER>：系统登录的用户名
*/
static char ftp_cmd[][6] = {"ABOR","ACCT","ALLO","APPE","CDUP","CWD","DELE","HELP","LIST","MODE","MKD","NLST","NOOP","PASS","PASV","PORT","PWD","QUIT","REIN","REST","RETR","RMD","RNFR"
,"RNTO","SITE","SMNT","STAT","STOR","STOU","STRU","SYST","TYPE","USER>"};

/*
响应代码 | 描述

110：新文件指示器上的重启标记
120：服务器准备就绪的时间（分钟数）
125：打开数据连接，开始传输
150：打开连接
200：成功
202：命令没有执行
211：系统状态回复
212：目录状态回复
213：文件状态回复
214：帮助信息回复
215：系统类型回复
220：服务就绪
221：退出网络
225：打开数据连接
226：结束数据连接
227：进入被动模式（IP 地址、ID 端口）
230：登录因特网
250：文件行为完成
257：路径名建立
331：要求密码
332：要求帐号
350：文件行为暂停
421：服务关闭
425：无法打开数据连接
426：结束连接
450：文件不可用
451：遇到本地错误
452：磁盘空间不足
500：无效命令
501：错误参数
502：命令没有执行
503：错误指令序列
504：无效命令参数
530：未登录网络
532：存储文件需要帐号
550：文件不可用
551：不知道的页类型
552：超过存储分配
553：文件名不允许
*/
static char ftp_status[][6] = {"110","120","125","150","200","202","211","212","213","214","215","220","221","225","226","227","230","250","257","331","332","350","421"
,"425","426","450","451","452","500","501","502","503","504","530","532","550","551","552","553"};

//ftp 协议分析
void process_ftp(const char *payload,db_ip_t *db_ip)
{
	int k0 = sizeof(ftp_cmd)/sizeof(ftp_cmd[0]);
	int k1 = sizeof(ftp_status)/sizeof(ftp_status[0]);
	debug("process_ftp ftp_cmd len:%d ftp_status len:%d",k0,k1);

	for(int i=0;i<k0;i++){
		if(strstr(payload,ftp_cmd[i])!=NULL){
			debug("Process FTP Protocol ftp_cmd match is %s",ftp_cmd[i]);
			goto end;
		}
	}

	for(int i=0;i<k1;i++){
		if(strstr(payload,ftp_status[i])!=NULL){
			debug("Process FTP Protocol ftp_status match is %s",ftp_status[i]);
			goto end;
		}
	}
	
	return;
	end:
		snprintf(db_ip->alert_msg,sizeof(db_ip->alert_msg),"%s","ftp");
}

void process_ftp_path1(const char *payload,db_ip_t *db_ip)
{
	int k0 = sizeof(ftp_cmd)/sizeof(ftp_cmd[0]);
	int k1 = sizeof(ftp_status)/sizeof(ftp_status[0]);
	//debug("process_ftp ftp_cmd len:%d ftp_status len:%d",k0,k1);

	for(int i=0;i<k0;i++){
		if(strstr(payload,ftp_cmd[i])!=NULL){
			snprintf(db_ip->alert_msg, sizeof(db_ip->alert_msg), "%s", ftp_cmd[i]);
			return;
		}
	}

	for(int i=0;i<k1;i++){
		if(strstr(payload,ftp_status[i])!=NULL){
			//debug("Process FTP Protocol ftp_status match is %s",ftp_status[i]);
			snprintf((char *)db_ip->alert_msg, sizeof(db_ip->alert_msg), "%s", ftp_status[i]);
			return;
		}
	}
}

//telnet 协议分析
void process_telnet(const char *payload,db_ip_t *db_ip)
{
	uchar byte_t = *(payload+1);
	enum _telnet_cmd telnet_cmd;
	for (telnet_cmd=NUL;telnet_cmd<=IAC;telnet_cmd++)
	{
		if(byte_t==telnet_cmd){
			debug("Process Telnet Protocol %02x",byte_t);
			goto end;
		}
	}

	enum _telnet_option telnet_option;
	for (telnet_option=TOPT_BIN;telnet_option<=TOPT_ERROR;telnet_option++)
	{
		if(byte_t==telnet_option){
			debug("Process Telnet Protocol %02x",byte_t);
			goto end;
		}
	}
	
	return;
	end:
		snprintf(db_ip->alert_msg,sizeof(db_ip->alert_msg),"%s","telnet");
}

//协议分析入口
void review_tcp(const char *payload,db_ip_t *db_ip)
{
	debug("review_process start");

	if(payload==NULL) return;

	//ftp
	process_ftp(payload,db_ip);
	//telnet
	process_telnet(payload,db_ip);
}

//协议分析入口
void review_udp(const char *payload,db_ip_t *db_ip)
{
	
}

//协议分析入口
/*void review_process(const char *payload,db_ip_t *db_ip)
{
	
}
*/
