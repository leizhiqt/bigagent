#ifndef _CHECK_DIR_
#define _CHECK_DIR_
#include <time.h>
 
#ifdef	__cplusplus
extern "C" {
#endif
#define  CHECK_DIR_SIZE (30000)
#define  FN_SIZE 1024
typedef struct my_struct
{
    char f_name[FN_SIZE];
    time_t ctime;
}f_struct,*p_f_struct;
 
//初始化文件缓冲队列
int neo_init_check_size();
 
//释放初始化的内存
int neo_close_check_size();
 
//获取缓冲队列首地址
p_f_struct neo_get_p_head();
 
//获取缓冲队列尾地址
p_f_struct neo_get_p_end();
 
//获取下一文件名地址
p_f_struct neo_get_p_next(p_f_struct p);
 
//显示缓冲区内容
void neo_print_f_name();
 
//将目录下所有文件名放入队列
int neo_check_dir(char *dir); 
 
//更具key将数组分为两部分
//p_f_struct partion(p_f_struct pstHead,p_f_struct pstLow,p_f_struct pstHigh);
p_f_struct partion(p_f_struct pstHead, p_f_struct pstEnd);
 
//对扫描到的文件按最后一次修改时间进行排序
//int quick_sort(p_f_struct pstHead, p_f_struct pstLoiw, p_f_struct pstHigh);
void quick_sort(p_f_struct pstHead, p_f_struct pstEnd);
 
#ifdef	__cplusplus
}
#endif
#endif
