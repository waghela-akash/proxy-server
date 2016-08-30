#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "proxy_parse.h"
using namespace std;

char buffer[4096];
map<string,string> request;
map<string,string> reply;
ParsedRequest *req;

int sendResponse(int newfd,int status){
	if(status<0){
		snprintf(buffer,4096,"HTTP/1.0 500 Internal Server Error\r\nConnection: close\r\n\r\n");
		send(newfd,buffer,strlen(buffer),0);
	}
	return 0;
}

int getResponse(int sockfd,int newfd){
	memset(buffer,0,sizeof(buffer));
	int size,tot=0;
	while((size=recv(sockfd,buffer,4096,0))>0){
		if(DEBUG)
			fwrite(buffer,1,size,stdout);
		send(newfd,buffer,size,0);
		debug("\nNum bytes read  %d\n",size);
		tot+=size;
		memset(buffer,0,sizeof(buffer));
	}

	debug("\nTOTAL Num bytes read  %d\n",tot);
	return 0;
}


int sendRequest(const char* msg, int newfd){
	int port = 80;
	if(req->port != NULL)
		port = atol(req->port);
	int sockfd = connectServer(req->host,port);
	send(sockfd,msg,strlen(msg),0);
	getResponse(sockfd, newfd);	
	close(sockfd);
	debug("Request Processed\n");
	return 0;
}

int recvTimeOut(int newfd){
	
	memset(buffer,0,sizeof(buffer));
	int bytes_recieved=0,now;
	while(1){
		cout<<"Got "<<bytes_recieved<<" "<<now<<endl;
		fd_set fds;
		int rc;
		struct timeval timeout;	
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(newfd, &fds); 
		rc = select(FD_SETSIZE,&fds, (fd_set *)NULL,(fd_set *)NULL,&timeout);
		if(rc == -1)
			return -1;
		else if(rc == 0)
			break;
		else if(rc == 1){
			now = recv(newfd,buffer+bytes_recieved,4096,0);
			bytes_recieved+=now;
		}
	}
	if(bytes_recieved<=0)
		return -1;
	cout<<buffer<<endl;
	return 1;
}

int getRequest(int newfd){
		
	if(recvTimeOut(newfd)<=0){
		//closeConnection(newfd);
		return 0;
	}
	debug("%s\n",buffer);
   
	int len = strlen(buffer); 
	//Create a ParsedRequest to use. This ParsedRequest
	//is dynamically allocated.
	req = ParsedRequest_create();
	if (ParsedRequest_parse(req, buffer, len) < 0) {
	   printf("parse failed\n");
	   return -1;
	}

	debug("Method:%s\n", req->method);
	debug("Host:%s\n", req->host);
	debug("Path:%s\n", req->path);
	debug("Buff:%s\n", req->buf);

	if (ParsedHeader_set(req, "Host", req->host) < 0){
		printf("set header key not work\n");
		return -1;
	}

	if (ParsedHeader_set(req, "Connection", "close") < 0){
		printf("set header key not work\n");
		return -1;
	}

	string s = "GET "+string(req->path)+" "+string(req->version)+"\r\n";

	int rlen = ParsedHeader_headersLen(req);
	char buf[rlen+1];
	if (ParsedRequest_unparse_headers(req, buf, rlen) < 0) {
		printf("unparse failed\n");
		return -1;
	}
	buf[rlen] ='\0';
	debug("%s\n",(s+string(buf)).c_str());
	sendRequest((s+string(buf)).c_str(), newfd);

	// Call destroy on any ParsedRequests that you
	// create once you are done using them. This will
	// free memory dynamically allocated by the proxy_parse library. 
	ParsedRequest_destroy(req);
	return 200;
}

