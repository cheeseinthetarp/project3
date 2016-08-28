#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>

#include "./make_log.h"
#include "mygetkey.h"

int paser(char* buf,char (*out)[64])
{
	if(buf == NULL || out == NULL)
	{
		return -1;
	}

	int in_index=0,out_index1=0,out_index2=0;

	while(buf[in_index] != '\0')
	{
		if(buf[in_index] == '\n')
		{
			out_index1++;
			out_index2 = 0;
			in_index++;
			continue;
		}

		out[out_index1][out_index2] = buf[in_index];
		in_index++;
		out_index2++;
	}

	return 0;
}

int paser2(char* buf,char* out)
{
	if(buf == NULL || out == NULL)
	{
		return -1;
	}

	int index=0;
	for(;buf[index] != '\0';)
	{
		if(buf[index] == ':')
			break;
		index++;
	}
	index +=2;

	int out_index=0;
	while(buf[index] != '\0')
	{
		out[out_index] = buf[index];
		out_index++;
		index++;
	}
	return 0;
}

void trimSpace(char* inStr)
{
	int i;
	for(i=0;inStr[i]!=0;i++)
	{
		if( inStr[i] == ' ' )
		{
			inStr[i] = '-';
			break;
		}
	}
}

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

	// char arg[128] = {0};
	// strcpy(arg,argv[2]);
	// int len = strlen(arg);
	// arg[len-1] = 0;
	// argv[2] = arg;

	// int fd = open("./log",O_RDWR);
	// char test[1024]={0};
	// sprintf(test,"%s\n%s",argv[1],argv[2]);
	// write(fd,test,strlen(test));

	pid = fork();

	if(pid == 0)
	{
		int fd = open("./log",O_RDWR);
		close(pipefd[0]);
		dup2(pipefd[1],STDOUT_FILENO);
		execlp("fdfs_file_info","fdfs_file_info",argv[1],argv[2],NULL);
		write(fd,"fdfs_file_info",strlen("fdfs_file_info"));
		close(fd);
		exit(-1);
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

		char buf[1024] = {0};
		ret = read(pipefd[0],buf,sizeof(buf));
		if(ret == -1)
		{
			printf("%d\n",__LINE__);
			exit(-1);
		}

		char out_buf[5][64] = {0};
		ret  = paser(buf,out_buf);

		// int i,j;
		// for(i=0;i<5;i++)
		// {
		// 	printf("%s\n",out_buf[i]);	
		// }

		char out_buf2[64]={0};
		char outCreateTime[64] = {0};
		ret = paser2(out_buf[1],out_buf2);
		paser2(out_buf[2],outCreateTime);
		trimSpace(outCreateTime);

		char ipa[128] = {0};
		sprintf(ipa,"http://%s/%s",out_buf2,argv[2]);
		//printf("%s\n",ipa);
		//LOG("test","fastdfs","get string[%s]",ipa);
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		char picurl[128] = {0};
		char* picurl_m[3] = {
			"http://192.168.75.129/static/file_png/jpg.png",
			"http://192.168.75.129/static/file_png/text.png",
			NULL
		};
		char* mime[3] = {
			"image/jpeg","text/plain",NULL
		};
		int i;
		for(i=0;picurl_m[i] != NULL;i++)
		{
			if( strcmp(argv[4],mime[i]) == 0 )
			{
				strcpy( picurl,picurl_m[i] );
			}
		}

		//===================> 将文件存储redis ======================
		redisContext* c = redisConnect("127.0.0.1", 6379);
		if ( c->err)
		{
			redisFree(c);
			printf("Connect to redisServer faile\n");
			exit(-1);
		}

		char command[1024] = {0};
		sprintf(command,"incr count");

		char count[5] = {0};
		set_key(c,command,count);

		memset(command,0,sizeof(command));
		sprintf(command,"set_key-count = %s\n",count);
		//write(fd,command,strlen(command));

		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
		 memset(command,0,sizeof(command));
		 sprintf(command,"zadd file_sortset %s %s",count,argv[3]);
		 set_key(c,command,NULL);
		//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

		memset(command,0,sizeof(command));
		sprintf(command,"hmset %s url %s descrip %s title_m %s title_s %s picurl_m %s kind %d pv %d hot %d",argv[3],ipa,outCreateTime,argv[3],"bbc",picurl,0,0,1);
		set_key(c,command,NULL);

		redisFree(c);

		//write(fd,command,sizeof(command));
		//close(fd);
		//===================> 将文件名存储redis ======================
	}
	else
	{
		printf("%d\n",__LINE__);
		exit(1);
	}
	
	return 0;
}

