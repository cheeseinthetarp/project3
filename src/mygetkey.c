#include "mygetkey.h"


void getKey_Value(redisContext* c,char* inStr,char* outValue)
{
	redisReply* r = (redisReply*) redisCommand(c,inStr);
	if(r->type == REDIS_REPLY_NIL)
	{
		strcpy(outValue,r->str);
	}
	else if(r->type == REDIS_REPLY_STRING)
	{
		strcpy(outValue,r->str);
	}
	
	freeReplyObject(r);
}
void getKey_array(redisContext* c,char* inStr,char (*out)[64],int* len)
{
	redisReply* r = (redisReply*) redisCommand(c,inStr);
	*len = r->elements;
	if(r->type == REDIS_REPLY_ARRAY)
	{
		int i;
		for(i=0;i < r->elements;i++)
		{
			if(r->element[i]->type == REDIS_REPLY_STRING)
			{
				//printf("value=(str)%s\n",r->element[i]->str);
				//strcpy(out[i],r->element[i]->str);
				memcpy(out[i],r->element[i]->str,strlen(r->element[i]->str));
			}
			else if(r->element[i]->type == REDIS_REPLY_INTEGER)
			{
				//printf("value=(long long int)%lld\n",r->element[i]->integer);
			}
		}
	}
	else if(r->type == REDIS_REPLY_INTEGER)
	{
		//printf("value=(long long int)%lld\n",r->integer);
	}
	else if(r->type == REDIS_REPLY_NIL)
	{
		strcpy(out[0],r->str);
	}

	freeReplyObject(r);
}

void set_key(redisContext* c,char* inStr,char* out)
{
	redisReply* r = (redisReply*) redisCommand(c,inStr);
	if(r->type == REDIS_REPLY_INTEGER)
	{
		if( out != NULL)
			sprintf(out,"%lld",r->integer);
	}
	if(r->type == REDIS_REPLY_STATUS)
	{
		//printf("set key status=%s\n",r->str);
	}
	freeReplyObject(r);
}


// int main()
// {
// 	redisContext* c = redisConnect("127.0.0.1", 6379);
// 	if ( c->err)
// 	{
// 		redisFree(c);
// 		printf("Connect to redisServer faile\n");
// 		return ;
// 	}

// 	printf("OK\n");

// 	char out[64] = {0};
// 	getKey_Value(c,"kkk",out);
// 	printf("value=%s\n",out);
	
// 	getKey_array(c,"lrange list 0 -1");
// 	getKey_array(c,"llen list");

// 	set_key(c,"set tarp cheese");
// 	redisFree(c);

// 	return 0;
// }




