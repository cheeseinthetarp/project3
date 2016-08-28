src = $(wildcard ./src/*.c)

out = $(patsubst %.c, %.o,$(src))

CPPFLAG = -I ./include

lib1 = -lhiredis
lib3 = -lhiredis -lfcgi -lm

pathbin = ./bin/

pathcli = ./src/myclient.o
pathinfo = ./src/myinfo.o ./src/mygetkey.o
pathdata = ./src/data.o ./src/cJSON.o ./src/mygetkey.o
pathupload=./src/upload_cgi.o ./src/util_cgi.o ./src/make_log.o ./src/mygetkey.o
	
obj:$(out) myclient myinfo data upload

myclient:$(pathcli)
	gcc $^ -o $(pathbin)$@ $(CPPFLAG)

myinfo:$(pathinfo)
	gcc  $^ -o $(pathbin)$@ $(CPPFLAG) $(lib1)

data:$(pathdata)
	gcc $^ -o $(pathbin)$@ $(CPPFLAG) $(lib3)

upload:$(pathupload)
	gcc $^ -o $(pathbin)$@ $(CPPFLAG) $(lib3)

%.o:%.c
	gcc -c $< -o $@ $(CPPFLAG)


