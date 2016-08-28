#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "./make_log.h"
#include <string.h>

int main(int argc,char* argv[])
{
	// printf("%s\n%s\n%s\n",argv[0],argv[1],argv[2]);
	// exit(1);
	if(argc < 3)
	{
		printf("Usage: (null) <config_file> <local_filename> [storage_ip:port] [\n");
		exit(1);
	}

	int res;
	pid_t pid;
	int pipefd[2];	

	res = pipe(pipefd);
	if(res == -1)
	{
		printf("%d\n",__LINE__);
		exit(1);
	}

	pid = fork();

	if(pid == 0)
	{
		int fd = open("./log",O_RDWR);
		close(pipefd[0]);
		dup2(pipefd[1],STDOUT_FILENO);
		execlp("fdfs_upload_file","fdfs_upload_file",argv[1],argv[2],NULL);
		write(fd,"fdfs_upload_file",strlen("fdfs_upload_file"));
		close(fd);
		exit(1);
	}
	else if(pid >0)
	{
		close(pipefd[1]);

		int status = 0;
		int ret = wait(&status);
		if(ret == -1)
		{
			printf("%d\n",__LINE__);
			exit(1);
		}
			
		if(WIFEXITED(status))
		{
			//printf("pargma true exit = %d\n",WEXITSTATUS(status));
		}
		if(WIFSIGNALED(status))
		{
			//printf("pargma signal exit = %d\n",WTERMSIG(status));
		}

		char buf[64] = {0};
		ret = read(pipefd[0],buf,sizeof(buf));
		if(ret == -1)
		{
			printf("%d\n",__LINE__);
			exit(1);
		}

		int len = strlen(buf);
		buf[len-1] = 0;
		printf("%s",buf);
		//LOG("test","fastdfs","get string[%s]",buf);
		//==============================> info =============================
		int pid;
		pid = fork();
		if(pid == 0)
		{
			int fd = open("./log",O_RDWR);
			execl("./bin/myinfo","myinfo","/etc/fdfs/client.conf",buf,argv[2],argv[3],NULL);
			write(fd,"myinfo",strlen("myinfo"));
			close(fd);
		}
		wait(NULL);
		//==============================> info =============================
	}
	else
	{
		printf("%d\n",__LINE__);
		exit(1);
	}
	
	return 0;
}

