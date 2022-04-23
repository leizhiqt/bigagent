#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include "ulog.h"
#include "times.h"

//时间->字符串
//size_t strftime(char *s, size_t max, const char *format,const struct tm *tm);

//strftime(s, 20, fmats, tm_now);

//字符串->时间
//char *strptime(const char *s, const char *format, struct tm *tm);

//struct tm stm;
//strptime(str_time, "%Y-%m-%d %H:%M:%S",&stm);

//按照不同的方式格式化当前times
void t_formats(char *s,const char*fmats)
{
	time_t now;
	time(&now);
	tm_now = localtime(&now);

	//时间->字符串
	strftime(s, 20, fmats, tm_now);
}

//获取当前时间
void t_ftime(char *tsname)
{
	t_formats(tsname,"%Y%m%d%H%M%S");
}
//获取当前日期
void t_fdate(char *s)
{
	t_formats(s,"%Y%m%d");
}

//获取当前时间
void t_stime(char *times)
{
	t_formats(times,"%Y-%m-%d %H:%M:%S");
}

//执行时间
int disTime()
{
	//执行一次处理的时间
	gettimeofday(&tv,NULL);
	if(tv_prv.tv_sec==0)
	{
		tv_prv = tv;
		return 1;
	}
	//秒 微秒
	//double second = (tv.tv_sec-tv_prv.tv_sec)+(double)(tv.tv_usec-tv_prv.tv_usec)/1000000;//秒

	//debug("Seconds:%ld Microseconds:%ld Seconds:%ld Microseconds:%ld  耗时秒:%lf",tv.tv_sec,tv.tv_usec,tv_prv.tv_sec,tv_prv.tv_usec,second);

	tv_prv = tv;

	return 0;
}

//192.168.1.70
int ntpdate(const char *f_ip)
{
	char ntpd[64];
	snprintf(ntpd,sizeof(ntpd),"sudo /usr/sbin/ntpdate %s &",f_ip);
	system(ntpd);
	return 0;
}

//秒:纳秒
void sys_ms(char *ms)
{
	struct timeval ts;
	gettimeofday(&ts,NULL);
	snprintf(ms,30,"%ld%ld",ts.tv_sec,ts.tv_usec);
}

//秒:纳秒
void now_ns(char* ns)
{
	sys_ms(ns);
}

//长整形time_t 转时间struct tm
void ltime(const time_t *timep)
{
	struct tm *p_tm = NULL;

	p_tm = localtime(timep);

	//pcap文件获取时间包头时间信息
	//p_tm = localtime(&header->ts.tv_sec);

/*
	//当前时间
	time_t nSeconds;
	time(&nSeconds);
	p_tm = localtime(nSeconds);
*/

	//系统日期,格式:YYYMMDD
	debug("%04d-%02d-%02d",p_tm->tm_year+1900, p_tm->tm_mon+1,p_tm->tm_mday);

	debug("hour: %u",p_tm->tm_hour);
}


//字符串格式化
void t_format_tt(const char *s_times,const char *s_format,const char *d_format,char *d_times,const int d_len)
{
	//size_t strftime(char *s, size_t max, const char *format,const struct tm *tm);
	//char *strptime(const char *s, const char *format, struct tm *tm);
	struct tm s_tm;
	strptime(s_times,s_format,&s_tm);
	strftime(d_times,d_len,d_format,&s_tm);
}
