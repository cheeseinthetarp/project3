#include "fcgi_config.h"

#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>

#include "fcgi_stdio.h"
#include "util_cgi.h"

#include "mygetkey.h"


int main ()
{
    char *file_buf = NULL;
    char boundary[256] = {0};
    char content_text[256] = {0};
    char filename[256] = {0};
    char fdfs_file_path[256] = {0};
    char fdfs_file_stat_buf[256] = {0};
    char fdfs_file_host_name[30] = {0};
    char fdfs_file_url[512] = {0};
    //time_t now;;
    


    while (FCGI_Accept() >= 0) {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        printf("Content-type: text/html\r\n"
                "\r\n");

        if (contentLength != NULL) {
            len = strtol(contentLength, NULL, 10);
        }
        else {
            len = 0;
        }

        if (len <= 0) {
            printf("No data from standard input\n");
        }
        else {
            int i, ch;
            char *begin = NULL;
            char *end = NULL;
            char *p, *q, *k;

            //==========> 开辟存放文件的 内存 <===========

            file_buf = malloc(len);
            if (file_buf == NULL) {
                printf("malloc error! file size is to big!!!!\n");
                return -1;
            }

            begin = file_buf;
            p = begin;
            for (i = 0; i < len; i++) {
                if ((ch = getchar()) < 0) {
                    printf("Error: Not enough bytes received on standard input<p>\n");
                    break;
                }
                //putchar(ch);
                *p = ch;
                p++;
            }

            //===========> 开始处理前端发送过来的post数据格式 <============
            //begin deal
            end = p;

            p = begin;

            //>>>>>>>>>>>>>>>>>>>>>>>>>get content-type>>>>>>>>>>>>>>>>
            char* pType_beg = strstr(p,"e: ");
            pType_beg += 3;
            if(pType_beg == NULL)
            {
                printf("find pType_beg eror\n");
                exit(-1);
            }
            char* pType_end  = strstr(p,"\r\n\r\n");
            if(pType_end == NULL)
            {
                printf("find pType_end eror\n");
                exit(-1);
            }
            char type[32] = {0};
            memcpy(type,pType_beg,(pType_end - pType_beg));
            printf("type=%s\n",type);

            //>>>>>>>>>>>>>>>>>>>>>>>>>get content-type>>>>>>>>>>>>>>>>

            //get boundary
            p = strstr(begin, "\r\n");
            if (p == NULL) {
                printf("wrong no boundary!\n");
                goto END;
            }

            strncpy(boundary, begin, p-begin);
            boundary[p-begin] = '\0';
           // printf("boundary: [%s]\n", boundary);

            p+=2;//\r\n
            //已经处理了p-begin的长度
            len -= (p-begin);

            //get content text head
            begin = p;

            p = strstr(begin, "\r\n");
            if(p == NULL) {
                printf("ERROR: get context text error, no filename?\n");
                goto END;
            }
            strncpy(content_text, begin, p-begin);
            content_text[p-begin] = '\0';
            //printf("content_text: [%s]\n", content_text);

            p+=2;//\r\n
            len -= (p-begin);

            //get filename
            // filename="123123.png"
            //           ↑
            q = begin;
            q = strstr(begin, "filename=");
            
            q+=strlen("filename=");
            q++;

            k = strchr(q, '"');
            strncpy(filename, q, k-q);
            filename[k-q] = '\0';

            trim_space(filename);
            //printf("filename: [%s]\n", filename);

            //get file
            begin = p;     
            p = strstr(begin, "\r\n");
            p+=4;//\r\n\r\n
            len -= (p-begin);

            begin = p;
            // now begin -->file's begin
            //find file's end
            p = memstr(begin, len, boundary);
            if (p == NULL) {
                p = end-2;    //\r\n
            }
            else {
                p = p -2;//\r\n
            }         
            //begin---> file_len = (p-begin)
            
            //>>>>>>>>>>>>>>>>>>>>>>>>>>>chongfude wenjian>>>>>>>>>>>>>>>>
            
            //>>>>>>>>>>>>>>>>>>>>>>>>>>>chongfude wenjian>>>>>>>>>>>>>>>>

            //=====> 此时begin-->p两个指针的区间就是post的文件二进制数据
            //======>将数据写入文件中,其中文件名也是从post数据解析得来  <===========
            char pathfile[528] = {0};
            sprintf(pathfile,"./file/%s",filename);
            int fd = 0;
            fd = open(pathfile, O_CREAT|O_WRONLY, 0644);
            if (fd < 0) {
                printf("open %s error\n", filename);
                goto END;
            }

            ftruncate(fd, (p-begin));
            write(fd, begin, (p-begin));
            close(fd);
            //===============> 将该文件存入fastDFS中,并得到文件的file_id <============
            //================ > 得到文件所存放storage的host_name <=================
            printf("filename=%s\n",filename);
            int pipefd[2] = {0};
            pipe(pipefd);

            int pid =  fork();
            if(pid == 0)
            {
                close(pipefd[0]);
                dup2(pipefd[1],STDOUT_FILENO);
                execl("./bin/myclient","myclient","/etc/fdfs/client.conf",filename,type,NULL);
                printf("myclient execl eror");
                exit(-1);
           }
            close(pipefd[1]);
            char pipe_buf[64] = {0};
            int pipe_res = read(pipefd[0],pipe_buf,sizeof(pipe_buf));
            if(pipe_res <= 0)
            {
                printf("pipebuf eror\n");
                exit(-1);
            }
            printf("pipe_buf=%s\nfilename=%s\n",pipe_buf,filename);
            wait(NULL);

            //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>读取redis >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            redisContext* c = redisConnect("127.0.0.1", 6379);
            if ( c->err)
            {
                redisFree(c);
                printf("Connect to redisServer faile\n");
                exit(-1);
            }
            char command[64] = {0};
            sprintf(command,"hset %s id %s",filename,pipe_buf);
            set_key(c,command,NULL);

            memset(command,0,sizeof(command));
            sprintf(command,"hget %s url",filename);
            char value[64] = {0};
            getKey_Value(c,command,value);
            printf("redis=%s\n",value);
            redisFree(c);
        //=====================================================================
END:

            memset(boundary, 0, 256);
            memset(content_text, 0, 256);
            memset(filename, 0, 256);
            memset(fdfs_file_path, 0, 256);
            memset(fdfs_file_stat_buf, 0, 256);
            memset(fdfs_file_host_name, 0, 30);
            memset(fdfs_file_url, 0, 512);

            free(file_buf);
            //printf("date: %s\r\n", getenv("QUERY_STRING"));
        }
    } /* while */

    return 0;
}
