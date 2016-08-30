#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include "server.h"
#include "response.h"
using namespace std;

int PORT=3490;

int main(int argc, char *argv[]){
	
	PORT=atol(argv[1]);
	
	int fd=bindSocket(PORT);
	while(1){
		int newfd = acceptConnection(fd);
		//if(fork==0){		
			if(newfd>0){
				int ret = getRequest(newfd);
				/*if(ret==0 || sendResponse(newfd,ret)==0){
					printf("Response Sent\n");
					closeConnection(newfd);
					break;
				}	*/	
				cout<<"Status "<<ret<<endl;		
			}
			closeConnection(newfd);
			//exit(0);
		//}
	}
	return 0;
}
