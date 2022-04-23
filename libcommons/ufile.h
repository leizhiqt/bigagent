#ifndef _UFILE_H_
#define _UFILE_H_

//去掉回车换行字符串
void brline(const char *ch,const int k);

//字符串是否文件已存在
int exist_line(const char *file_name,const char *buf);

int save_txt(const char *file_name,const char *buf);

int save_append(const char *file_name,const char *buf);

int save_flush(const char *file_name,const char *buf);

int binary_append(const char *file_name,const char *buf,const int len);

//压缩打包
void tarz(const char *infile, const char *outfile);

//删除目录
int rm_task(const char * tpath);

int mkdir_p(const char *fullpath,const uint len);

/*
	cp_file					复制函数
	char *destination_path	目标路径
	char *source_path		源路径
*/
int cp_file(char *destination_path,char *source_path);

/*
	const char *path	文件路径
*/
ulong get_file_size(const char *path);

//读取单行数据
int read_line(const char *fpath,char *buf,const int buf_size);

#endif
