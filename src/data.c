
#include "fcgi_config.h"
#include "fcgi_stdio.h"
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>


#include "cJSON.h"
#include "mygetkey.h"

extern char **environ;

int main ()
{

    //int count=0;
    while (FCGI_Accept() >= 0)
    {
        char *contentLength = getenv("CONTENT_LENGTH");
        int len;

        FCGI_printf("Content-type: application/json\r\n\r\n");


        if (contentLength != NULL)
        {
            len = strtol(contentLength, NULL, 10);
        }
        else
        {
            len = 0;
        }

        if (len <= 0)
        {

            int fd = open("./log",O_RDWR);
           char *getStr = getenv("QUERY_STRING");
           char* pId = strstr(getStr,"Id=");
           char* pId_end = strstr(getStr,"&count");
           pId += 3;

            char fromId[3] = {0};
            memcpy(fromId,pId,(pId_end - pId) );
            write(fd,fromId,strlen(fromId));
            //close(fd);

           // >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>读取redis>>>>>>>>>>>>>>>>>>>>>>>>>
            redisContext* c = redisConnect("127.0.0.1", 6379);
            if ( c->err)
            {
                redisFree(c);
                FCGI_printf("Connect to redisServer faile\n");
                exit(-1);
            }

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            // char vCom[32] = {0};
            // sprintf(vCom,"hget fromId %s",fromId);
            // char Access_count[5] = {0};
            // getKey_Value(c,vCom,Access_count);

            // char start[5] = {0};
            // char stop[5] = {0};
             // if(strcmp(Access_count,"nil") == 0 )
             // {              
             //            memset(vCom,0,sizeof(vCom));
             //            sprintf(vCom,"hset fromId %s %s",fromId,"1");
             //            set_key(c,vCom,NULL);
             //            strcpy(start,"0");
             //            strcpy(stop,"8");
             // }
             // else
             // {
             //            int account = atoi(Access_count);
             //            account += 1;
             //            sprintf(start,"%d",(account * 8-8) );
             //            sprintf(stop,"%d",(account * 8));
             //            memset(vCom,0,sizeof(vCom));
             //            sprintf(vCom,"hset fromId %s %d",fromId,account);
             //            set_key(c,vCom,NULL);
             // }
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
            char start[5] = {0};
            char stop[5] = {0};
            int nstart = atoi(fromId);
            if(nstart == 0)
            {
                        strcpy(start,fromId);
                        strcpy(stop,"8");
            }
            else
            {
                        strcpy(start,fromId);
                        nstart += 8;
                        sprintf(stop,"%d",nstart);
            }

           
            char command[1024] = {0};
            char sortset_arr[10][64] = {0};
            int elements=0;
            sprintf(command,"zrange file_sortset %s %s",start,stop);
            getKey_array(c,command,sortset_arr,&elements);
            write(fd,command,strlen(command));
            close(fd);
            memset(command,0,sizeof(command));

            // char* static_field[10] = {
            // 	"title_s","pv","descrip","id","picurl_m","kind","hot","url","title_m",NULL
            // }
            

            int i;   
            char *out;
        	cJSON *root,*pArray;
        	root=cJSON_CreateObject();//创建js对象
        	pArray = cJSON_CreateArray();//创建js数组
        	cJSON_AddItemToObject(root,"games",pArray);

            for(i=0;i<elements;i++)
            {

            	cJSON *fmt;		                       		            
	cJSON_AddItemToArray(pArray, fmt=cJSON_CreateObject());//在项目上添加项目

            	char title_s[32]={0};
            	sprintf(command,"hget %s %s",sortset_arr[i],"title_s");
            	getKey_Value(c,command,title_s);
            	memset(command,0,sizeof(command));
            	cJSON_AddStringToObject(fmt,"title_s", title_s);
            	
            	char id[128] = {0};
            	sprintf(command,"hget %s %s",sortset_arr[i],"id");
            	getKey_Value(c,command,id);
            	memset(command,0,sizeof(command));
            cJSON_AddStringToObject(fmt,"id", id);

            char skind[2] = {0};
            int kind;
            sprintf(command,"hget %s %s",sortset_arr[i],"kind");
            	getKey_Value(c,command,skind);
            	kind = atoi(skind);
            	memset(command,0,sizeof(command));
	            cJSON_AddNumberToObject(fmt,"kind", kind);

	            char title_m[128] = {0};
            	sprintf(command,"hget %s %s",sortset_arr[i],"title_m");
            	getKey_Value(c,command,title_m);
            	memset(command,0,sizeof(command));
	            cJSON_AddStringToObject(fmt,"title_m", title_m);
	            
	             char descrip[128] = {0};
            	sprintf(command,"hget %s %s",sortset_arr[i],"descrip");
            	getKey_Value(c,command,descrip);
            	memset(command,0,sizeof(command));
	            cJSON_AddStringToObject(fmt,"descrip", descrip);


	             char picurl_m[128] = {0};
            	sprintf(command,"hget %s %s",sortset_arr[i],"picurl_m");
            	getKey_Value(c,command,picurl_m);
            	memset(command,0,sizeof(command));
	            cJSON_AddStringToObject(fmt,"picurl_m", picurl_m);


	             char url[128] = {0};
            	sprintf(command,"hget %s %s",sortset_arr[i],"url");
            	getKey_Value(c,command,url);
            	memset(command,0,sizeof(command));
	            cJSON_AddStringToObject(fmt,"url", url);

	             char spv[2] = {0};
	             int pv;
            	sprintf(command,"hget %s %s",sortset_arr[i],"pv");
            	getKey_Value(c,command,spv);
            	pv = atoi(spv);
            	memset(command,0,sizeof(command));
	            cJSON_AddNumberToObject(fmt,"pv", pv);


	             char shot[2] = {0};
	             int hot;
            	sprintf(command,"hget %s %s",sortset_arr[i],"hot");
            	getKey_Value(c,command,shot);
            	hot = atoi(shot);
            	memset(command,0,sizeof(command));
	            cJSON_AddNumberToObject(fmt,"hot", hot);          
	        }

 			out=cJSON_Print(root);
		    FCGI_printf("%s\n",out);//此时out指向的字符串就是JSON格式的了
		    free(out);//释放空间
	        cJSON_Delete(root);

	        redisFree(c);
            /* int i;
             for(i=0;i<elements;i++)
             {
             	char test[128] = {0};
             	sprintf(test,"%s==%d\n",hash[i],elements);
             	write(fd,test,strlen(test));
             }*/
        }
        else
        {
        }
    }


    return 0;
}