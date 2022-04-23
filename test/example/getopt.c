#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<getopt.h>

extern char *optarg;

extern int optind, opterr, optopt;

//optind——再次调用 getopt() 时的下一个 argv 指针的索引。

//optarg——指向当前选项参数（如果有）的指针。

//optopt——最后一个未知选项。

int main(int argc,char **argv,char **envp)
{
/*
	struct option {
		//选项名称
		const char *name;

		int		 has_arg;
		//参数标志
			//no_argument/0			没有参数
			//required_argument/1	需要一个参数
			//optional_argument/2	一个可选参数

		int		*flag;//指定如何返回一个较长的选项。当这个指针为空的时候，函数直接将val的数值从getopt_long的返回值返回出去，当它非空时，val的值会被赋到flag指向的整型数中，而函数返回值为0
		int		val;		//用于指定函数找到该选项时的返回值，或者当flag非空时指定flag指向的数据的值。
	};
*/
	//长选项结构体数组
	static struct option long_opts[] = {
		{"help",	no_argument,		NULL,	'h'},
		{"version",	no_argument,		NULL,	'v'},
		{"create",	optional_argument,	0,		10},
		{"conf",	required_argument,	0,		'c'},
		{0,			0,					0,		0}
	};

	//argc argv ：直接从main函数传递而来
	//short_opts：短选项字符串。如”n:v"，这里需要指出的是，短选项字符串不需要‘-’，而且但选项需要传递参数时，在短选项后面加上“：”。
	//long_opts/long_options：struct option 数组，用于存放长选项参数。
	//long_ind：用于返回长选项在long_opts结构体数组中的索引值，用于调试。一般置为NULL

	int required=0;

	//用于接收字符选项
	int opt;

	char *short_opts = "h::v::c:";
	int long_ind = 0;

	//opt==-1 参数结束
	while((opt = getopt_long(argc, argv, short_opts, long_opts, &long_ind))!= -1)
	{
		//printf("opt=%c\toptarg=%s\toptind=%d\t*argv=%s\n", opt,optarg,optind,*(argv+optind));
		//printf("opt=%c\toptarg=%s\toptind=%d\t*argv=%s\n", opt,optarg,optind,*(argv+optind));

		/*switch (opt) {	//获取参数解析
			case 0:
				break;

			case '0':
			case '1':
			case '2':
				break;
			case 'a':
				printf("选项: a\n");
				break;
			case '?':
				break;
			default:
				printf("?? getopt 返回字符代码 0%o ??\n", opt);
		}
		*/
	}

	//printf("opt=%d\toptarg=%s\toptind=%d\t*argv=%s\n", opt,optarg,optind,*(argv+optind));

	printf("optind=%d\topterr=%d\toptopt=%d\toptarg=%s\targc=%d\n",optind,opterr,optopt,optarg,argc);

	if (optind < argc) {
		printf("non-option ARGV-elements: ");
		while (optind < argc)
			printf("%s ", *(argv+optind++));
		printf("\n");
	}

	if(!required)
	{
		printf("Error: output name must be specified\n");
		return 1;
	}

	return 0;
}
