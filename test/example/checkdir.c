#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <dirent.h>
#include "check_dir.h"
 
static p_f_struct neo_p_head = NULL;
static p_f_struct neo_p_end = NULL;
 
//初始化文件缓冲队列
int neo_init_check_size()
{
	if(neo_p_head != NULL)
		return -1;
	neo_p_head = (p_f_struct)malloc(sizeof(f_struct) * CHECK_DIR_SIZE);
	if(neo_p_head == NULL)
		return -1;
	memset(neo_p_head,0,sizeof(f_struct) * CHECK_DIR_SIZE);
	neo_p_end = neo_p_head;
	return 0;
}
 
//释放初始化的内存
int neo_close_check_size()
{
	//p_f_struct p = neo_p_head;
	if(neo_p_head != NULL)
	{
		free(neo_p_head);
		neo_p_head = NULL;
		neo_p_end = NULL;
		return 0;
	}
	return -1;
}
 
//获取缓冲队列首地址
p_f_struct neo_get_p_head()
{
	if(neo_p_head->f_name[0] == 0)
		return NULL;
	return neo_p_head;
}
 
//获取缓冲队列尾地址
p_f_struct neo_get_p_end()
{
	if(neo_p_head->f_name[0] == 0)
		return NULL;
	neo_p_end = neo_p_head;
	while((neo_p_end + 1)->f_name[0] != 0)
		neo_p_end ++;
	return neo_p_end;
}
 
//获取下一文件名地址
p_f_struct neo_get_p_next(p_f_struct p)
{
	p++;
	if(p->f_name[0] == 0)
		return NULL;
	return p;
}
 
//显示缓冲区内容
void neo_print_f_name()
{
	int i = 0;
	p_f_struct p = neo_p_head;
	while(p->f_name[0] != 0)
	{
		printf("ctime : [%ld]	  file name : [%s]\n",p->ctime,p->f_name);
		p ++;
		i ++;
	}
	printf("sum = %d\n",i);
}
 
//将目录下所有文件名放入队列
int neo_check_dir(char *dir) 
{
	DIR * dp;
	struct dirent *dent;
	struct stat st;
	char fn[1024];
	//char fn1[1024];
	if(neo_p_head == NULL)
	{
		printf("neo_p_head is NULL,need neo_init_check_size()!");
		return -1;
	}
	p_f_struct p = neo_p_head;
	//p_f_struct q = neo_p_head;
 
	dp = opendir(dir);
	if (!dp) {
		printf("无法打开文件夹[%s]\n", dir);
 
		return -2;
	}
	while ((dent = readdir(dp)) != NULL) {
		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
			continue;
		}
		//MFLIST *mf;
		sprintf(fn, "%s/%s", dir, dent->d_name);
		if (stat(fn, &st) == 0) {
			if (S_ISDIR(st.st_mode)) {
				neo_check_dir(fn);
 
			} else if (S_ISREG(st.st_mode)) {
				snprintf(p->f_name,FN_SIZE,"%s",fn);
				p->ctime = st.st_ctime;
				//neo_p_end = p;
				p ++;
				if(p - neo_p_head >= (CHECK_DIR_SIZE - 1))
					return -1;
				/*p = (p_f_struct)malloc(sizeof(f_struct));
				  sprintf(p->f_name,"%s",fn);
				  p->ctime = st.st_ctime;
				  if(neo_p_head == NULL)
				  {
				  neo_p_head = p;
				  neo_p_head->next = NULL;
				  neo_p_head->front = neo_p_head;
				  continue;
				  }
				  if(p->ctime <= neo_p_head->ctime)
				  {
				  p->next = neo_p_head;
				  neo_p_head = p;
				  neo_p_head->front = neo_p_head;
				  continue;
				  }
				  q = neo_p_head->next;
				  while(q != NULL)
				  {
				  if(p->ctime <= q->ctime)
				  {
				  q->front->next = p;
				  p->next = q;
				  break;
				  }
				  q = q->next;
				  }
				  if(q == NULL)
				  {
				  q = p;
				  q->next = NULL;
				  }*/
 
 
 
			} 
		} else {
			printf("can't stat %s\n", fn);
		}
	}
	closedir(dp);
	return 0;
}
 
//更具key将数组分为两部分
p_f_struct partion(p_f_struct pstHead, p_f_struct pstEnd)
{
	f_struct temp_struct;
	memcpy(&temp_struct, pstHead, sizeof(f_struct));
	while(pstHead != pstEnd)
	{
		while((pstHead < pstEnd) && (pstEnd->ctime >= temp_struct.ctime))
			pstEnd --;
		if(pstHead < pstEnd){
	//		printf("%s,%ld\n",pstEnd->f_name,pstEnd->ctime);
			memcpy(pstHead, pstEnd, sizeof(f_struct));
			pstHead ++;
		}
		while((pstHead < pstEnd) && (pstHead->ctime <= temp_struct.ctime))
			pstHead ++;
		if(pstHead < pstEnd){
  //		  printf("%s,%ld\n",pstHead->f_name,pstHead->ctime);
			memcpy(pstEnd, pstHead, sizeof(f_struct));
			pstEnd --;
		}
	}
//			printf("%s,%ld\n",temp_struct.f_name,temp_struct.ctime);
	memcpy(pstHead, &temp_struct, sizeof(f_struct));
	return pstHead;
}
//对扫描到的文件按最后一次修改时间进行排序
void quick_sort(p_f_struct pstHead, p_f_struct pstEnd)
{
	if(pstHead < pstEnd)
	{
		p_f_struct temp_Head = pstHead;
		p_f_struct temp_End = pstEnd;
		p_f_struct pstTemp = partion(temp_Head, temp_End);
		quick_sort(pstHead, pstTemp - 1);
		quick_sort(pstTemp + 1, pstEnd);
	}
}

int main(int argc, char** argv, char *envp[]){
	return 0;
}
