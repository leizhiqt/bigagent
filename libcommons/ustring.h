#ifndef _USTRING_H_
#define _USTRING_H_

//字符串节点
typedef struct _ustring_node{
	link_node_t node;
	char *data;
	int len;
}ustring_node_t;

//动态字符串
//typedef struct _link_list ustring;

//动态字符串
typedef struct _ustring{
	link_list_t link_list;
	uint size;
	char *buf;
}ustring;

//创建动态字符串对象
ustring* ustring_new();
//增加字符串
void ustring_append(ustring *u_string,const char* str,const int len);
//生成最终字符串
char* ustring_rebuild(ustring *u_string);
//释放资源
void ustring_free(ustring *u_string);
//求字符串总长度
int ustring_usize(ustring *u_string);

/*binary_to_hex
	*二进制字节数组 转换为 16进制的字符串
	*char* hexs			16进制的字符串
	*const int hexsmax		16进制的字符串长度
	*const char* binary	二进制字节数组
	*const int smax		二进制字节数组长度 
	*return int 0:转换成功 非0:转换失败
*/
int binary_to_hex(char* hexs,const int hexsmax, const char* binary, const int smax);

/*hex_to_binary
	*16进制的字符串转换为二进制字节数组
	*char* binaries 	二进制字节数组
	*const char* hexs	16进制的字符串
	*return int 0:转换成功 非0:转换失败
*/
int hex_to_binary(char* binary, const char* hexs);

//字符串操作
//返回str2第一次出现在str1中的位置(下表索引),不存在返回-1
int indexOf(char *str1,char *str2);

//返回str1中最后一次出现str2的位置(下标),不存在返回-1
int lastIndexOf(char *str1,char *str2);

//截取src字符串中,从下标为start开始到end-1(end前面)的字符串保存在dest中(下标从0开始)
void substring(char *dest,char *src,int start,int end);

//字符替换
int strn_repc(const char *str,const char c0, const char c1,const int fn);

#endif
