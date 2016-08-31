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
ParsedRequest *req;			// Stores parsed Request from the client

// This function is only invoked in case of an ERROR in proxt server
// Returns to client "Internal Server Error" and closes the connection
int sendResponse(int newfd,int status){
	if(status<0){
		snprintf(buffer,4096,"HTTP/1.0 500 Internal Server Error\r\nConnection: close\r\n\r\n");
		send(newfd,buffer,strlen(buffer),0);
	}
	return 0;
}

// Getting Response from the requested HOST and then sending
// it to client in batch of 4096 bytes.
// The response is untamperd and "no checks" are made as specified in
// the assignment
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

// Sending request to the server on the sepicified port
// If no PORT is specified then PORT=80
int sendRequest(const char* msg, int newfd){
	int port = 80;
	if(req->port != NULL)
		port = atol(req->port);
	int sockfd = connectServer(req->host,port);
	send(sockfd,msg,strlen(msg),0);
	getResponse(sockfd, newfd);	
	close(sockfd);
	return 0;
}

// Recieving a request until TimeOUT using select()
// Code Courtsey PDF provided in Project1
int recvTimeOut(int newfd){

	memset(buffer,0,sizeof(buffer));
	int bytes_recieved=0,now;
	fd_set fds;
	int rc;
	struct timeval timeout;	
	timeout.tv_sec = 5;
	timeout.tv_usec = 0;
	FD_ZERO(&fds);
	FD_SET(newfd, &fds);
	while(1){
		rc = select(FD_SETSIZE,&fds, (fd_set *)NULL,(fd_set *)NULL,&timeout);
		if(rc == -1)
			return -1;
		else if(rc == 0){
			cout<<"Got "<<bytes_recieved<<" "<<now<<endl;
			break;
		}
		else if(rc == 1){
			now = recv(newfd,buffer+bytes_recieved,4096-bytes_recieved ,0);
			bytes_recieved+=now;
		}
	}
	if(bytes_recieved<=0)
		return -1;
	cout<<buffer<<endl;
	return 1;
}

// Getting request from client, the size of the request is 
// bounded by 4096 bytes
int getRequest(int newfd){
		
	// Recieving a GET request till "\r\n\r\n" is read or timeout
	// In case of timeout it will automatically give parsing error
	// and 500 Internal Server Error will be reported as specified
	// TimeOut is set by setsockopt() during connection
	int bytes_recieved=0,now;
	while((now=recv(newfd,buffer+bytes_recieved,4096-bytes_recieved,0))>0){
		bytes_recieved+=now;
		//cout<<buffer<<endl;
		if(strcmp(buffer+(strlen(buffer)-4),"\r\n\r\n")==0)
			break;
	}	

	// If nothing is recived close the connection
	if(bytes_recieved<=0){
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

	// Sets the Host the request
	if (ParsedHeader_set(req, "Host", req->host) < 0){
		printf("set header key not work\n");
		return -1;
	}

	// Sets the connection to close as specified in the instructions
	if (ParsedHeader_set(req, "Connection", "close") < 0){
		printf("set header key not work\n");
		return -1;
	}

	// Building the GET request to the specified host
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

