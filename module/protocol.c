#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include "ulog.h"
#include "ufile.h"
#include "protocol.h"

/*

void send_eof(int sockfd)
{
	char eof[4]={'\n','E','O','F'};
	int k=send(sockfd,eof,4,0);
	//debug("send_eof k:%d",k);
}

void send_nop(int sockfd)
{
	char buf[4];

	int k=-1;
	memset(buf,'\0',sizeof(buf));
	sprintf(buf,"%s","EOF\0");
	if((k=send(sockfd,buf,4,0))<1)
	{
		//debug("send_nop k:%d",k);
	}
	usleep(1000*20);
	debug("send_nop k:%d",k);
}

int send_rep_txt(const int sockfd,const char *buf)
{
	int k=0;
	if((k=send(sockfd,buf,strlen(buf),0))<1)
	{
		debug("send_rep_txt response ERROR:%d buf:%s",k,buf);
		return -1;
	}
	debug("send_rep_txt response Success:%d buf:%s",k,buf);

	send_eof(sockfd);
	//debug("send_nop k:%d",k);
	return 0;
}

int send_sf(int csockfd,const char *fullpath)
{
	char buf[BUFFER_SIZE+1];

	int fd = open(fullpath,O_RDONLY,0644);
	if(fd ==-1)
	{
		printf("open error\n");
		goto end;
	}
	//debug("send_rep:%d",fd);
	int n = 0;
	int RET = -1;

	lseek(fd,0L,SEEK_SET);
	while((n = read(fd,buf,BUFFER_SIZE)) > 0) 
	{
		//debug("send_rep buf:%s",buf);
		//清理堆数据
		RET= send(csockfd,buf,n,0);
		memset(buf,'\0',BUFFER_SIZE+1);

		//debug("send_rep RET:%d",RET);
		//发送
		if(RET<0) 
		{
			debug("Send File:%s Failed.",fullpath);
			goto end;
		}
	}
	debug("send_rep end RET:%d",RET);


	close(fd);//关闭文件

	//释放资源
	end:
		send_eof(csockfd);

	debug("send_sf:%s to %d",fullpath,csockfd);
	return 0;
}

//privte
//01:100:1000:02
*/

//01:100:1000:02
int recv_cmd_check(const char *req,thclient_t *thclient)
{
	
}

/*
//public API
//协议API
int recv_protocol(const char *req)
{
	int bsize = strlen(req);
	if(!(bsize>0)) return 0;

	char buf[bsize+1];

	int k = 0;
	memset(buf,'\0',bsize+1);
	memcpy(buf,req,bsize);

	char *token=strtok(buf,":");
	while(token!=NULL){
		//debug(" recv_cmd k:%d token:%s",k,token);
		switch(k)
		{
			case 0:
				if(strcmp(token,"01"))
					return 0;
			case 1:
				break;
			case 2:
				break;
			case 3:
				if(strcmp(token,"02"))
					return 0;
			default:
				break;
		}

		token=strtok(NULL,":");
		k++;
	}
	debug("recv_cmd protocol:%s",buf);
	return 1;
}
*/

//协议命令处理
int recv_cmd(const char *req,thclient_t *thclient)
{
	int bsize = 63;

	char buf[bsize+1];

	int protocol = 0;
	int cmd = 100;

	char task_id[bsize+1];
	char cmd2[bsize+1];
	char rep_cmd2[bsize+1];
	char reps[bsize+1];
	
	int k = 0;
	memset(buf,'\0',bsize+1);
	memcpy(buf,req,bsize);

	memset(reps,'\0',bsize+1);
	memset(rep_cmd2,'\0',bsize+1);

	debug(" recv_cmd cmd:%s",buf);
	
	char *token=strtok(buf,":");
	while(token!=NULL){
		//debug(" recv_cmd ERROR k:%d token:%s",k,token);
		switch(k)
		{
			case 0://开头
				if(!strcmp(token,"01"))
					protocol=1;
				else
					protocol=0;

				strcat(reps,token);
				break;
			case 1://命令
				cmd = atoi(token);

				strcat(reps,":");
				strcat(reps,token);
				break;
			case 2://任务号
				//snprintf(task_match->task_id,sizeof(task_match->task_id),"%s",token);

				strcat(reps,":");
				strcat(reps,token);
				break;
			case 3://参数
				snprintf(cmd2,sizeof(cmd2),"%s",token);

				strcat(reps,":");
				strcat(reps,"%s");
				break;
			case 4://结束
				if(!strcmp(token,"02"))
					protocol=protocol&1;
				else
					protocol=protocol&0;

				strcat(reps,":");
				strcat(reps,token);
				break;
			default:
				break;
		}

		token=strtok(NULL,":");
		k++;
	}

	debug(" recv_cmd k:%d protocol:%d",k,protocol);

	if(!protocol || k!=5)
		return -1;

	int c=0;
	//debug(" recv_cmd cmd:%d",cmd);
	switch(cmd)
		{
			case 100://01:100:1000:02
				//100 返回图片
				k = 0;
				//debug(" recv_cmd task_match->task_id:%s",task_match->task_id);

				//memset(&(task_match->ucase_id),'\0',sizeof(task_match->ucase_id));
				//cvSendPack(thclient);

				break;
			case 101:
				//101 基准对比坐标
				k = 0;
				//debug(" recv_cmd cmd2:%s",cmd2);
/*
				struct CvRect mh_rect;
				mh_rect.x=0;
				mh_rect.y=0;
				mh_rect.width=0;
				mh_rect.height=0;

				token=strtok(cmd2,",");
				while(token!=NULL){
					//debug(" recv_cmd cmd2 k:%d token:%s",k,token);
					switch(k)
					{
						case 0:
							c=atoi(token);//摄像头编号
							strcat(rep_cmd2,token);
							//debug(" x:%d",c);
							break;
						case 1:
							mh_rect.x=atoi(token);
							strcat(rep_cmd2,",");
							strcat(rep_cmd2,token);
							//debug(" x:%d",atoi(token));
							if(mh_rect.x<0) return -3;
							break;
						case 2:
							//debug(" y:%d",atoi(token));
							mh_rect.y=atoi(token);
							strcat(rep_cmd2,",");
							strcat(rep_cmd2,token);
							if(mh_rect.y<0) return -4;
							break;
						case 3:
							//debug(" width:%d",atoi(token));
							mh_rect.width=atoi(token);
							strcat(rep_cmd2,",");
							strcat(rep_cmd2,token);
							if(mh_rect.width<0) return -5;
							break;
						case 4:
							//debug(" height:%d",atoi(token));
							mh_rect.height=atoi(token);
							strcat(rep_cmd2,",");
							strcat(rep_cmd2,token);
							if(mh_rect.height<0) return -6;
							break;
						default:
							break;
					}

					token=strtok(NULL,",");
					k++;
				}
				//debug(" recv_cmd cmd2 k:%d",k);

				memset(&(task_match->ucase_id),'\0',sizeof(task_match->ucase_id));
				cvSendFile(thclient,c,&mh_rect);

				strcat(rep_cmd2,",1");
				//strcat(rep_cmd2,",0");

				//返回信息
				memset(buf,'\0',bsize+1);
				snprintf(buf,sizeof(buf),reps,rep_cmd2);
				strcat(buf,"\n");

				send_rep_txt(thclient->csockfd,buf);
*/
				break;
			case 102://01:102:74:3:02
				//102 开始任务
				k = 0;
				//debug(" recv_cmd cmd2:%s",cmd2);
				/*token=strtok(cmd2,",");
				while(token!=NULL){
					//debug("recv_cmd cmd2 k:%d token:%s",k,token);
					switch(k)
					{
						case 0:
							debug("recv_cmd cmd2 ucase_id:%s",token);
							snprintf(task_match->ucase_id,sizeof(task_match->ucase_id),"%s",token);

							strcat(rep_cmd2,token);
							strcat(rep_cmd2,",1");
							//debug("recv_cmd cmd2 task_id:%s",task_match->task_id);
							break;
						default:
							break;
					}

					token=strtok(NULL,",");
					k++;
				}
				if(strcmp(" ",task_match->ucase_id))
				{
					task_match->s_task = 1;

					char savepath[80];
					memset(savepath,'\0',80);

					for(int i=0;i<task_match->ccamras;i++)
					{
						match_video = *(task_match->match_pool+i);
						set_save_fpath(savepath,sizeof(savepath),match_video->vid,_task_match.task_id,_task_match.ucase_id,0);
						//debug("set_save_fpath ccamras=%d",task_match->ccamras);
					}

					//返回信息
					memset(buf,'\0',bsize+1);
					snprintf(buf,sizeof(buf),reps,rep_cmd2);
					strcat(buf,"\n");

					send_rep_txt(thclient->csockfd,buf);
					//debug("recv_cmd buf:%s",buf);
				}*/
				break;
			case 103://01:103:74:3:02
				//103 关闭任务
				k = 0;
				debug(" recv_cmd cmd2:%s",cmd2);
/*
				token=strtok(cmd2,",");
				while(token!=NULL){
					//debug("recv_cmd cmd2 k:%d token:%s",k,token);
					switch(k)
					{
						case 0:
							debug("recv_cmd cmd2 ucase_id:%s length=%d",token,strlen(token));
							snprintf(task_match->ucase_id,sizeof(task_match->ucase_id),"%s",token);
							strcat(rep_cmd2,token);
							strcat(rep_cmd2,",1");
							//debug("recv_cmd cmd2 task_id:%s",task_match->task_id);
							break;
						default:
							break;
					}

					token=strtok(NULL,",");
					k++;
				}

				if(!strcmp(" ",task_match->ucase_id)) task_match->s_task = 0;

				//返回信息
				memset(buf,'\0',bsize+1);
				snprintf(buf,sizeof(buf),reps,rep_cmd2);
				strcat(buf,"\n");
				send_rep_txt(thclient->csockfd,buf);

				//清理任务
				debug(" recv_cmd cmd2:%d %d",k,!strcmp(" ",task_match->ucase_id));

				memset(&(task_match->task_id),'\0',sizeof(task_match->task_id));
				memset(&(task_match->ucase_id),'\0',sizeof(task_match->ucase_id));
				//task_remove(task_match->task_id);
				return -1;*/
			case 104:
				//算法选择
				//01:104:74:0,1:02
				k = 0;
				//debug(" recv_cmd cmd2:%s",cmd2);
				/*int f=0;
				int l=-1;
				token=strtok(cmd2,",");
				while(token!=NULL){
					//debug(" recv_cmd cmd2 k:%d token:%s",k,token);
					switch(k)
					{
						case 0:
							c=atoi(token);//摄像头编号
							strcat(rep_cmd2,token);
							//debug(" x:%d",c);
							break;
						case 1:
							f=atoi(token);
							strcat(rep_cmd2,",");
							strcat(rep_cmd2,token);
							//debug(" x:%d",atoi(token));
							break;
						case 2:
							l=atoi(token);
							strcat(rep_cmd2,",");
							strcat(rep_cmd2,token);
							//debug(" y:%d",atoi(token));
							break;
						case 3:
							//debug(" width:%d",atoi(token));
							break;
						case 4:
							//debug(" height:%d",atoi(token));
							break;
						default:
							break;
					}

					token=strtok(NULL,",");
					k++;
				}
				//debug(" recv_cmd cmd2 k:%d",k);
				match_video=get_mvideo(thclient->task_match,c);

				if(l>0 && l<11) match_video->level = l;

				match_video_run(match_video,f);

				strcat(rep_cmd2,",1");
				//strcat(rep_cmd2,",0");

				//返回信息
				memset(buf,'\0',bsize+1);
				snprintf(buf,sizeof(buf),reps,rep_cmd2);
				strcat(buf,"\n");

				send_rep_txt(thclient->csockfd,buf);*/
				break;
			case 105://01:105:196:4:02
				//105 下载异常文件
				k = 0;
				//debug(" recv_cmd cmd2:%s",cmd2);
				/*
				token=strtok(cmd2,",");
				while(token!=NULL){
					//debug(" recv_cmd cmd2 k:%d token:%s",k,token);
					switch(k)
					{
						case 0:
							snprintf(task_match->ucase_id,sizeof(task_match->ucase_id),"%s",token);
							//debug("recv_cmd cmd2 task_id:%s",task_match->task_id);
							send_pack(thclient);
							break;
						default:
							//debug("cmd2 file_name:%s",token);
							//send_nop(task_match->csockfd);
							break;
					}

					token=strtok(NULL,",");
					k++;
				}
				//debug(" recv_cmd cmd2 return k:%d",k);
				//return -1;
				break;
			case 106://01:106:196:0:02
				//106	删除任务
				k = 0;
				*/
				break;
			default:
				break;
		}
	//debug(" recv_cmd ERROR k:%d token:%s",k,token);
	return k;
}

//MSG_DONTWAIT 阻塞
//MSG_PEEK 非阻塞 不清空TCP缓存
//0

//协议握手
//阻塞接收命令
int recv_shake_hand(thclient_t *thclient)
{
	int csockfd = thclient->csockfd;

	char buf[BUFFER_SIZE+1];
	int k = 0;
	int *web_end = (int *)buf;
	int wsh = 1;
	while(1)
	{
		//阻塞接收
		memset(buf,'\0',BUFFER_SIZE+1);
		k = recv(csockfd, buf, BUFFER_SIZE, 0);
		debug(" received data:%s k=%d",buf,k);

		if(k<0)
		{
			debug("客户端关闭 received error k:%d csockfd:%d",k,csockfd);
			break;
		}
		else if(k==0)
		{
			// 由于是非阻塞的模式,所以当errno为EAGAIN时,表示当前缓冲区已无数据可读
			// 在这里就当作是该次事件已处理处.
			if(errno == EAGAIN)
			{
				//debug("errno EAGAIN",__FILE__,__LINE__);
				continue;
			}
			//等待客户端关闭
			debug("客户端关闭 received error k:%d csockfd:%d",k,csockfd);
			break;
		}
		else
		{
			//去掉换行符
			brline(buf,k);

			//合法协议处理
			recv_cmd(buf,thclient);
			continue;
		}
		//释放CPU
		usleep(200*1000);
	}

	debug(" recv_shake_hand close k:%d",k);
	return k;
}
