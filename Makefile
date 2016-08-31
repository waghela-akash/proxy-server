CC=g++
CFLAGS= -g -Wall -Werror

all: proxy

proxy: proxy.cpp
	$(CC) proxy.cpp -o proxy

clean:
	rm -f proxy *.o

tar:
	tar -cvzf ass1.tgz proxy.cpp response.h server.h README Makefile proxy_parse.c proxy_parse.h
