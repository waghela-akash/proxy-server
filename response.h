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
#include <sys/types.h>
#include <unistd.h>
#include "proxy_parse.h"
#include "proxy_parse.c"
using namespace std;

char buffer[4096];
map<string,string> request;
map<string,string> reply;
int dbug=1;
ParsedRequest *req;

int getResponse(int sockfd,int newfd){
	memset(buffer,0,sizeof(buffer));
	int size,tot=0;
	while((size=recv(sockfd,buffer,4096,0))>0){
		fwrite(buffer,1,size,stdout);
		send(newfd,buffer,size,0);
		cout<<"\nNum bytes read  "<<size<<endl;
		tot+=size;
		memset(buffer,0,sizeof(buffer));
	}
	cout<<"\nTOTAL Num bytes read  "<<tot<<endl;
	if(dbug)
		printf("Response Sent\n");
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
	if(dbug)
		printf("Request Processed closed\n");
	return 0;
}

int getRequest(int newfd){
		
	memset(buffer,0,sizeof(buffer));
	int bytes_recieved = recv(newfd,buffer,4096,0);
	if(bytes_recieved<=0){
		//closeConnection(newfd);
		return 0;
	}
	printf("%s\n",buffer);
   
	int len = strlen(buffer); 
	//Create a ParsedRequest to use. This ParsedRequest
	//is dynamically allocated.
	req = ParsedRequest_create();
	if (ParsedRequest_parse(req, buffer, len) < 0) {
	   printf("parse failed\n");
	   return -1;
	}

	printf("Method:%s\n", req->method);
	printf("Host:%s\n", req->host);
	printf("Path:%s\n", req->path);
	printf("Buff:%s\n", req->buf);

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
	cout<<(s+string(buf)).c_str()<<endl;
	sendRequest((s+string(buf)).c_str(), newfd);

	// Call destroy on any ParsedRequests that you
	// create once you are done using them. This will
	// free memory dynamically allocated by the proxy_parse library. 
	ParsedRequest_destroy(req);
	return 200;
}

