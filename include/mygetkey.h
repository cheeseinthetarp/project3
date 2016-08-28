#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hiredis/hiredis.h>

void getKey_Value(redisContext* c,char* inStr,char* outValue);
void getKey_array(redisContext* c,char* inStr,char (*out)[64],int* len);
void set_key(redisContext* c,char* inStr,char* out);