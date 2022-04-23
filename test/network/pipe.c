#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

enum PIPES { PIPE_READ, PIPE_WRITE };   /* Constants 0 and 1 for PIPE_READ and PIPE_WRITE */

int sync_pipe[2];                       /* pipe used to send messages from child to parent */
int data_pipe[2];                       /* pipe used to send data from child to parent */

int main()
{
	pid_t pid;
	char buffer[32];

	memset(buffer, 0, 32);
	if(pipe(sync_pipe) < 0)
	{
		printf("Failed to create pipe!\n");
		return 0;
	}

	static int k=0;
	char w_buf[64];

	if(0 == (pid = fork()))
	{
		while(1){
			close(2);
			dup2(sync_pipe[PIPE_WRITE],2);

			close(sync_pipe[PIPE_READ]);

			snprintf(w_buf,sizeof(w_buf),"P%d",k++);
			if(write(2,w_buf, strlen(w_buf))>=0)
			{
				//fprintf(stderr,"write:%s",w_buf);
			}
			sleep(1);
		}
	}

//0 是一个文件描述符，表示标准输入(stdin)
//1 是一个文件描述符，表示标准输出(stdout)
//2 是一个文件描述符，表示标准错误(stderr)
//dup2(sync_pipe[PIPE_WRITE],1);
//重定向到stderr

	while(1){
		//重定向
		//dup2(sync_pipe[PIPE_WRITE],2);

		close(sync_pipe[PIPE_WRITE]);
		if(read(sync_pipe[PIPE_READ], buffer, 32) > 0)
		{
			fprintf(stderr,"%s",buffer);
		}
		sleep(1);
	}

	waitpid(pid,NULL,0);

	return 1;
} 
