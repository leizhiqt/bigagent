#ifndef _U_LOG_
#define _U_LOG_

#define		LOG_LINE_SIZE	1024*4	//4k

#define snprintf(str, size, format, arg...)	\
		snprintf((char *) str, size, format, ##arg)

int log_get_level();

void log_set_level(int level);

void logInit(const char *s,int level);

void logsk(const int sk);

//printfs("%s\t%d\t[ERROR]:%s",__FILE__,__LINE__, "test");
void printfs(FILE *stream,const char *fmt, ...);

//debug("%s","debug test");
//#define debug(format, arg...) printfs("%s\t%d\t[DEBUG]:"format,__FILE__,__LINE__, ##arg)
#define debug(format, arg...) 						\
		({											\
			const int buf_size=128;\
			if(strlen(format)>buf_size-28) 					\
			{						\
				printf("%s\t%d\t[DEBUG]:format par is maxed\n",__FILE__,__LINE__);	\
			}	else {		\
				char ftr[buf_size];						\
				memset(ftr,'\0',buf_size);				\
				strcat(ftr,"%s\t\%d\t[DEBUG]:");		\
				strcat(ftr,(format));					\
				printfs(stdout,ftr,__FILE__,__LINE__, ##arg);	\
			}\
		})

#define error(format, arg...) 						\
		({											\
			const int buf_size=128;\
			if(strlen(format)>buf_size-28) 					\
			{						\
				printf("%s\t%d\t[ERROR]:format par is maxed\n",__FILE__,__LINE__);	\
			}	else {		\
				char ftr[buf_size];						\
				memset(ftr,'\0',buf_size);				\
				strcat(ftr,"%s\t\%d\t[ERROR]:");		\
				strcat(ftr,(format));					\
				printfs(stderr,ftr,__FILE__,__LINE__, ##arg);	\
			}\
		})

void printbytes(const char *bytes,const int n);

#endif

