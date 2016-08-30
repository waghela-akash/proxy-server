#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <sys/wait.h>
#include "server.h"
#include "response.h"
#include "proxy_parse.c"
using namespace std;

int PORT=3490;

int main(int argc, char *argv[]){
	
	PORT=atol(argv[1]);
	
	int fd=bindSocket(PORT);
	int child=0;
	while(1){
		int newfd = acceptConnection(fd);
		if(newfd>0 && fork()==0){		
			int ret = getRequest(newfd);
			if(ret<0){
				sendResponse(newfd, ret);
			}		
			debug("Status %d\n",ret);		
			closeConnection(newfd);
			exit(0);
		}		
		else if(newfd>0){
			child++;
			closeConnection(newfd);
		}
		debug("Number of child %d\n",child);
		if(child >= 20){
			while(child--){
				int status;
				wait(&status);
			}
		}
	}
	return 0;
}
