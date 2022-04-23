#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

#include "ulog.h"
#include "dtype.h"
#include "linklist.h"
#include "ustring.h"
#include "conf.h"

#define LINE_SIZE 512

static char *cnf_sign = "=";
static char _conf_file[128];

static link_list_t *u_conf=NULL;

//配置初始化
link_list_t* uconf_new()
{
	link_list_t *u_conf = (link_list_t *) malloc(sizeof(link_list_t));
	link_list_init(u_conf);
	return u_conf;
}

const char* uconf_get(link_list_t *u_conf,const char *key)
{
	if(u_conf==NULL) return NULL;

	link_node_t *p = u_conf->head;
	uconf_node_t *uconf_node_p;

	while(p!=NULL){
		uconf_node_p = container_of(p,uconf_node_t,node);
		//debug("ustring_free ustr_node_p:%p",ustr_node_p);

		if(!strcmp(uconf_node_p->key,key))
		{
			return uconf_node_p->value;
		}

		p = p->next;
	}
	return NULL;
}

int uconf_set(link_list_t *u_conf,const char *key,const int key_len,const char *value,const int value_len)
{
	if(u_conf==NULL) return -1;

	link_node_t *p = u_conf->head;
	uconf_node_t *uconf_node_p;

	while(p!=NULL){
		uconf_node_p = container_of(p,uconf_node_t,node);
		//debug("ustring_free ustr_node_p:%p",ustr_node_p);

		if(!strcmp(uconf_node_p->key,key))
		{
			free(uconf_node_p->value);

			uconf_node_p->value = malloc(sizeof(char)*value_len+1);
			memcpy(uconf_node_p->value,value,value_len);
			uconf_node_p->value_len=value_len;
			*(uconf_node_p->value+value_len)='\0';
			return 1;
		}

		p = p->next;
	}
	return 0;
}

int uconf_add(link_list_t *u_conf,const char *key,const int key_len,const char *value,const int value_len)
{
	if(u_conf==NULL) return -1;

	if(uconf_set(u_conf,key,key_len,value,value_len))	return 1;

	uconf_node_t *uconf_node_p= (uconf_node_t*) malloc(sizeof(uconf_node_t));

	uconf_node_p->key = malloc(sizeof(char)*key_len+1);
	memcpy(uconf_node_p->key,key,key_len);
	uconf_node_p->key_len=key_len;
	*(uconf_node_p->key+key_len)='\0';

	uconf_node_p->value = malloc(sizeof(char)*value_len+1);
	memcpy(uconf_node_p->value,value,value_len);
	uconf_node_p->value_len=value_len;
	*(uconf_node_p->value+value_len)='\0';

	//debug("ustring_append uconf_node_p:%p",uconf_node_p);
	link_list_add_last(u_conf,&(uconf_node_p->node));

	return 0;
}

//释放资源
void uconf_destroy(link_list_t *u_conf)
{
	if(u_conf==NULL) return;
	//释放化队列
	link_list_destroy(u_conf,uconf_node_t);

	free(u_conf);

	//int size = link_list_size(u_conf);
	//debug("size=%d",u_conf->size);
}

int uconf_load(link_list_t *u_conf,const char *conf_path)
{
	FILE *fp;
	//debug("open %s",conf_path);
	fp = fopen(conf_path,"r+");
	if(fp == NULL)
	{
		 error("open FALID:%s ",conf_path);
		 return -1;
	}
	//debug("conf_path %s",conf_path);
	char linebuf[LINE_SIZE];

	char key[LINE_SIZE];
	char value[LINE_SIZE];
	int key_len,value_len;

	while(fgets(linebuf,LINE_SIZE,fp) != NULL)
	{
		//debug("config_linebuf:%s",linebuf);
		if(linebuf[0]=='#' || linebuf[0]=='\n') continue;

		int i = indexOf(linebuf,cnf_sign);
		substring(key,linebuf,0,i);
		key_len = i+1;

		substring(value,linebuf,i+1,strlen(linebuf)-1);
		value_len = strlen(value);

		uconf_add(u_conf,key,key_len,value,value_len);

		//debug("%s\t=%s #size:%d",key,value,u_conf->size);
	}
	fclose(fp);

	return 0;
}

//全局配置实现
int conf_enable()
{
	if(link_list_size(u_conf)>0)
	{
		const char *val = conf_get(CONF_ENABLE);
		if(val!=NULL && !strcmp(val,"Yes"))
			return 1;
	}
	return 0;
}

//加载配置文件内容
int conf_load(const char *conf_path)
{
	int ret = -1;

	if(u_conf==NULL)
		u_conf=uconf_new();

	if((ret=uconf_load(u_conf,conf_path))!=0)
		return ret;

	memset(_conf_file,'\0',sizeof(_conf_file));
	snprintf(_conf_file,sizeof(_conf_file),"%s",conf_path);

	debug("conf_load %s",_conf_file);

	return 0;
}

//conf_key NULL 遍历所有键值
const char* conf_get(const char *conf_key)
{
	if(link_list_size(u_conf)<1) return NULL;

	return uconf_get(u_conf,conf_key);
}

//设置配置键值
void conf_set(const char *conf_key,const char *conf_value)
{
	if(link_list_size(u_conf)<1) return;

	uconf_set(u_conf,conf_key,strlen(conf_key),conf_value,strlen(conf_value));
}

/*
 *添加修改文件（当配置文件中存在标记字段，则进行修改，若不存在则进行添加）
 *
 *输入参数：1，配置文件路径 2，匹配标记 3，替换或添加的内容
 *
 */

/*
int conf_update(char *conf_path,char *conf_name,char *conf_value)
{
	char cnf_buf[LINE_SIZE];
	memset(cnf_buf,'\0',sizeof(cnf_buf));
		
	char linebuf[LINE_SIZE];
	
	char lkey[LINE_SIZE];
	char lval[LINE_SIZE];
	char lread[LINE_SIZE];
	
	FILE *f;
	f = fopen(conf_path,"r+");
	if(f == NULL)
	{
		printf("OPEN CONFIG FALID\n");
		return 0;
	}

	int on_line = 0;
	
	while(fgets(linebuf,LINE_SIZE,f) != NULL)
	{   
		 //printf("config_linebuf:%s\n",linebuf);
		 snprintf(lread,strlen(linebuf),"%s",linebuf);
		//printf("%s||%s\n",lkey,conf_name);

		if(!strcmp(lkey,conf_name)){
			 strcat(cnf_buf,conf_name);
			  strcat(cnf_buf,"=");
			  strcat(cnf_buf,conf_value);
			  strcat(cnf_buf,"\n");
			  
			  on_line=1;
			 //break;
		}else{
			 strcat(cnf_buf,lread);
			 strcat(cnf_buf,"\n");
		}
	}
	remove(conf_path);
	fclose(f);

	if(!on_line){
		 strcat(cnf_buf,conf_name);
		 strcat(cnf_buf,"=");
		 strcat(cnf_buf,conf_value);
		 strcat(cnf_buf,"\n");
		 
		 on_line=1;
	}
	
	FILE *fp;
	fp = fopen(conf_path,"w+");
	if(fp == NULL)
	{
		 printf("OPEN CONFIG FALID\n");
		 return 2;
	}
	//fseek(fp,0,SEEK_SET);
	
	//printf("cnf_buf>%s\n",cnf_buf);
	fputs(cnf_buf,fp);
	fclose(fp);
}
*/

/*
int conf_delete(char *conf_path,char *conf_name)
{
	char cnf_buf[LINE_SIZE];
	memset(cnf_buf,'\0',sizeof(cnf_buf));
		
	char linebuf[LINE_SIZE];
	
	char lkey[LINE_SIZE];
	char lval[LINE_SIZE];
	char lread[LINE_SIZE];
	
	FILE *f;
	f = fopen(conf_path,"r+");
	if(f == NULL)
	{
		printf("OPEN CONFIG FALID\n");
		return 0;
	}

	int on_line = 0;
	
	while(fgets(linebuf,LINE_SIZE,f) != NULL)
	{   
		 //printf("config_linebuf:%s\n",linebuf);
		 snprintf(lread,strlen(linebuf),"%s",linebuf);
			char *token=strtok(linebuf,cnf_sign);
			int k=0;
		while(token!=NULL){
			//debug(" recv_cmd ERROR k:%d token:%s",k,token);
			switch(k)
			{
				case 0://key
					 printf("%s=",token);
					 snprintf(lkey,strlen(token),"%s",token);
					break;
				case 1://value
					 printf("%s\n",token);
					 snprintf(lval,strlen(token),"%s",token);
					break;
				default:
					break;
			}

			token=strtok(NULL,cnf_sign);
			k++;
		}
		
		if(strcmp(lkey,conf_name) && k==2){
			 strcat(cnf_buf,lread);
			 strcat(cnf_buf,"\n");
		}
	}
	remove(conf_path);
	fclose(f);

	FILE *fp;
	fp = fopen(conf_path,"w+");
	if(fp == NULL)
	{
		 printf("OPEN CONFIG FALID\n");
		 return 2;
	}
	//fseek(fp,0,SEEK_SET);
	
	//printf("cnf_buf>%s\n",cnf_buf);
	fputs(cnf_buf,fp);
	fclose(fp);
}
*/

/*
int conf_append(char *conf_path,char *conf_name,char *conf_value)
{
	char cnf_buf[LINE_SIZE];
	memset(cnf_buf,'\0',sizeof(cnf_buf));

	char *cnf_sign = "=";

	FILE *fp;
	fp = fopen(conf_path,"a+");
	if(fp == NULL)
	{
		 printf("OPEN CONFIG FALID\n");
		 return 0;
	}

	strcat(cnf_buf,conf_name);
	strcat(cnf_buf,cnf_sign);
	strcat(cnf_buf,conf_value);
	strcat(cnf_buf,"\n");

	fputs(cnf_buf,fp);
	fclose(fp);
}
*/

/*void conf_test()
{
	debug("conf_test run finish");
}*/
