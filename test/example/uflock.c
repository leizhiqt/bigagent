#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char** argv, char *envp[]){

	//lock set
	struct flock lock = {0};
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	//unlock get
	struct flock unlock = {0};
	unlock.l_type = F_UNLCK;
	unlock.l_whence = SEEK_SET;
	unlock.l_start = 0;
	unlock.l_len = 0;

	int fd;
	char *lock_file="/tmp/1023.pid";
	int ret=-1;

	//文件检测
	if(access(lock_file,F_OK|R_OK)==-1)
	{
		//创建文件
		if((fd = open(lock_file,O_WRONLY|O_CREAT))==-1)
		{
			printf("Error open %s\n",lock_file);
			return -1;
		}

		if((ret = fcntl(fd, F_SETLK, &lock))==-1)
		{
			printf("lock %d\n",ret);
		}

		sleep(100);
		return 0;
	}

	if((fd = open(lock_file,O_RDWR))==-1)
	{
		printf("Error open %s\n",lock_file);
		return -1;
	}

	try:
	if((ret = fcntl(fd, F_SETLK, &lock))==-1)
	{
		printf("lock %d\n",ret);
		sleep(100);
		goto try;
	}
	printf("lock %d\n",ret);
	sleep(100);

	if((ret = fcntl(fd, F_SETLK, &unlock))==-1)
	{
		printf("unlock %d\n",ret);
	}
	printf("unlock %d\n",ret);

	close(fd);

	return 0;
}
