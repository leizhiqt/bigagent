/********************************** 
 * @author	leizhifesker@icloud.com
 * @date	2017/03/07
 * Last update:	2017/03/07
 * License:	LGPL
 * 
 **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>

#include "ulog.h"
#include "dtype.h"
#include "linklist.h"
#include "ustring.h"

//==============
ustring* ustring_new()
{
	ustring *u_string = (ustring*) malloc(sizeof(ustring));
	link_list_init(&u_string->link_list);

	u_string->size=0;
	u_string->buf=NULL;

	return u_string;
}

void ustring_append(ustring *u_string,const char *str,const int len)
{
	//debug("str=%s",str);
	//debug("ustr_node_p->data=%s",ustr_node_p->data);
	//debug("ustring_append len=%d",len);

	ustring_node_t *ustr_node_p= (ustring_node_t *) malloc(sizeof(ustring_node_t));

	ustr_node_p->data = malloc(sizeof(char)*len+1);
	memcpy(ustr_node_p->data,str,len);
	ustr_node_p->len=len;
	*(ustr_node_p->data+len)='\0';

	//统计字符串总数
	u_string->size +=len;
	//debug("malloc %p->%p",ustr_node_p,&(ustr_node_p->node));

	link_list_add_last(&u_string->link_list,&(ustr_node_p->node));
}

//字符串总长度
int ustring_usize(ustring *u_string)
{
	//debug("u_string->size:%d",u_string->size);
	return u_string->size;
}

char* ustring_rebuild(ustring *u_string)
{
	if(u_string==NULL) return NULL;
	
	if(u_string->buf!=NULL)
	{
		return u_string->buf;
	}

	//debug("u_string->size:%d",u_string->size);

	u_string->buf = (char*) malloc(u_string->size+1);
	memset(u_string->buf,'\0',u_string->size+1);

	link_node_t *p = u_string->link_list.head;
	ustring_node_t *ustr_node_p;

	int offset=0;
	while(p!=NULL){
		ustr_node_p = container_of(p,ustring_node_t,node);
		//debug("ustring_rebuild %p->%p",ustr_node_p,p);

		if(ustr_node_p->data==NULL) break;

		memcpy(u_string->buf+offset,ustr_node_p->data,ustr_node_p->len);

		//debug("offset=%d dp->len=%d dp->data %s",offset,dp->len,dp->data);
		//debug("u_string->buf %s",u_string->buf+offset);
		//debug("u_string->buf %s",u_string->buf);

		offset += ustr_node_p->len;

		p = p->next;
	}
	//debug("%s",u_string->buf);

	return u_string->buf;
}

void ustring_free(ustring *u_string)
{
	if(u_string==NULL) return;

	ustring_node_t *ustr_node_p;

	link_node_t *p = u_string->link_list.head;

	while(p!=NULL){
		//获取节点指针
		ustr_node_p = container_of(p,ustring_node_t,node);
		//debug("free %p->%p",ustr_node_p,p);

		//释放节点元素
		if(ustr_node_p->data!=NULL)
		{
			free(ustr_node_p->data);
			ustr_node_p->data=NULL;
		}
		ustr_node_p->len=0;

		p = p->next;
	}
	u_string->size=0;
	if(u_string->buf!=NULL)
		free(u_string->buf);

	//释放化队列
	link_list_destroy(&u_string->link_list,ustring_node_t);

	//debug("free :%p",u_string);
	free(u_string);
}

/*binary_to_hex
	*二进制字节数组 转换为 16进制的字符串
	*char* hexs			16进制的字符串
	*const int hexsmax		16进制的字符串长度
	*const char* binary	二进制字节数组
	*const int smax		二进制字节数组长度 
	*return int 0:转换成功 非0:转换失败
*/
int binary_to_hex(char* hexs,const int hexsmax, const char* binary, const int smax)
{
	const char* chs_hex = "0123456789ABCDEF";
	int k = 0;
	int i=0,j=0;

	for(i=0,j=0;i<smax;i++,j+=2)
	{
		if(j>=hexsmax) return 1;

		k = (binary[i] & 0xf);
		hexs[i*2+1] = chs_hex[k];

		k = ((binary[i]>>4) & 0xf);
		hexs[i*2] = chs_hex[k];
		
		//debug(" %c %c",hexs[i*2],hexs[i*2+1]);
	}
	return 0;
}

/*hex_to_binary
 *16进制的字符串转换为二进制字节数组
 *char* binaries 	二进制字节数组
 *const char* hexs	16进制的字符串
 *return int 0:转换成功 非0:转换失败
*/
int hex_to_binary(char* binary, const char* hexs)
{
	int RET = -1;

	int i=0;
	int maxs = strlen(hexs);

	if(maxs%2 != 0 ) return RET;
	//unsigned int ascii = 65535;
	for (i = 0; i < maxs/2 ; i++)
	{
		RET = sscanf(hexs+i*2, "%02X", (unsigned int*)(binary+i));
		//if(*(unsigned int*)(s+i) > 255) break;
		//debug("RET=%d %c %c %c",RET,*(hexs+i),*(hexs+i+1),*(s+i));
	}
	//debug("i=%d",i);
	*(binary+i)= '\0';
	return 0;
}

/*a2x
	*16进制的字符转换为二进制字符
	*char ch			16进制的字符
	*return char 二进制字符
*/
char a2x(char ch)
{
	switch(ch)
	{
		case '1':
			return 1;
		case '2':
			return 2;
		case '3':
			return 3;
		case '4':
			return 4;
		case '5':
			return 5;
		case '6':
			return 6;
		case '7':
			return 7;
		case '8':
			return 8;
		case '9':
			return 9;
		case 'A':
		case 'a':
			return 10;
		case 'B':
		case 'b':
			return 11;
		case 'C':
		case 'c':
			return 12;
		case 'D':
		case 'd':
			return 13;
		case 'E':
		case 'e':
			return 14;
		case 'F':
		case 'f':
			return 15;
		default:
			break;
	}
	return 0;
}

//字符串操作
//	返回str2第一次出现在str1中的位置(下表索引),不存在返回-1
int indexOf(char *str1,char *str2)
{
	char *p=str1;
	int i=0;
	p=strstr(str1,str2);
	if(p==NULL)
		return -1;
	else{
		while(str1!=p)
		{
			str1++;
			i++;
		}
	}
	return i;
}

//	返回str1中最后一次出现str2的位置(下标),不存在返回-1
int lastIndexOf(char *str1,char *str2)
{
	char *p=str1;
	int i=0,len=strlen(str2);
	p=strstr(str1,str2);
	if(p==NULL)return -1;
	while(p!=NULL)
	{
		for(;str1!=p;str1++)i++;
		p=p+len;
		p=strstr(p,str2);
	}
	return i;
}

//截取src字符串中,从下标为start开始到end-1(end前面)的字符串保存在dest中(下标从0开始)
void substring(char *dest,char *src,int start,int end)
{
	int i=start;
	if(start>strlen(src))return;
	if(end>strlen(src))
		end=strlen(src);
	while(i<end)
	{	
		dest[i-start]=src[i];
		i++;
	}
	dest[i-start]='\0';
	return;
}

//	字符替换	str_replace_char
int strn_repc(const char *str,const char c0, const char c1,const int fn)
{
	//如果串相等，则直接返回
	if(str==NULL) return -1;

	char *p = (char *)str;
	int n=0;
	do
	{
		if(*(p+n)==c0) *(p+n)=c1;
	}while((p+n)!=NULL && (n++<fn));

	return 0;
}
