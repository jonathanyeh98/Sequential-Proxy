/*
 * proxy.c - A Simple Sequential Web proxy
 *
 * Course Name: 14:332:456-Network Centric Programming
 * Assignment 2
 * Student Name:______________________
 * 
 * IMPORTANT: Give a high level description of your code here. You
 * must also provide a header comment at the beginning of each
 * function that describes what that function does.
 */ 

#include "csapp.h"

/*
 * Function prototypes
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, char *uri, int size);

/* 
 * main - Main routine for the proxy program 
 */
int main(int argc, char **argv)
{
	
	int port;
	int listenfd;
	int serverfd;
	int clientfd;
	int clientlen;
	struct sockaddr_in clientaddr;
	
	rio_t rio;
	
	//set up log file
	
	int flags = O_RDWR | O_APPEND | O_CREAT;
	int fd = open("proxy.log", flags, S_IRWXU);
	
	
	
	
	char buffer[MAXBUF];

    /* Check arguments */
    if (argc != 2) {
	fprintf(stderr, "Usage: %s <port number>\n", argv[0]);
	exit(0);
    }

    port = atoi(argv[1]);
	
	//socket(),bind(),listen() wrapper defined in csapp.c
	listenfd = Open_listenfd(port);

	//start the loop
	while(1){
		clientlen = sizeof(clientaddr);
		
		clientfd = Accept(listenfd, (SA*) &clientaddr, (socklen_t*) &clientlen);
		
		
		Rio_readinitb(&rio,clientfd);
		Rio_readlineb(&rio,buffer,MAXBUF);
		
		//parse request
		char command[MAXLINE];
		char url[MAXLINE];
		char protocol[MAXLINE];
				
		sscanf(buffer, "%s %s %s", command, url, protocol);
		
		printf("%s %s %s\n", command, url, protocol);
		//extracting port number and domain name from url
		char * domainName;
		int serverport = 80;
		
		char * tempString = &url[0];
		char * tempString2;
		
		//we need to check if url has protocol exlicitly stated, if not skip if block
		if(strstr(tempString, "//") != NULL){
			tempString = strstr(tempString, "/");
			//remove the first 2 slashes  
			tempString += 2;
			tempString2 = strstr(tempString, "/");
			//remove the trailing slash
			
		}
		*tempString2 = 0;

		//if there is no ":", then tempString is just the domain name
		if((strstr(tempString, ":")) == NULL){

			domainName = tempString;
		}
		else{

			domainName = strtok(tempString, ":");
			tempString2 = strtok(NULL,":");
			serverport = atoi(tempString2);
		}

		printf("Server domain: %s\n", domainName);
		printf("Server port: %d\n", serverport);
		
		//now we send the request to the server
		serverfd = Open_clientfd(domainName,serverport);

		Rio_writen(serverfd, buffer, MAXBUF);

		
		int n;
		
		n = (int) Rio_readn(serverfd,buffer,MAXLINE);

		Rio_writen(clientfd,buffer,n);
		bzero(buffer, MAXBUF);
		
		
				
		//write to log
		char * logentry = (char *) Malloc(sizeof(char));
		format_log_entry(logentry, &clientaddr, url, n);
		write(fd,logentry,strlen(logentry));
		printf("logged: %s\n\n", logentry);
		
		free(logentry);
		close(clientfd);
		close(serverfd);
	}
    exit(0);
    
}

/*
 * format_log_entry - Create a formatted log entry in logstring. 
 * 
 * The inputs are the socket address of the requesting client
 * (sockaddr), the URI from the request (uri), and the size in bytes
 * of the response from the server (size).
 */
void format_log_entry(char *logstring, struct sockaddr_in *sockaddr, 
		      char *uri, int size)
{
    time_t now;
    char time_str[MAXLINE];
    unsigned long host;
    unsigned char a, b, c, d;

    /* Get a formatted time string */
    now = time(NULL);
    strftime(time_str, MAXLINE, "%a %d %b %Y %H:%M:%S %Z", localtime(&now));

    /* 
     * Convert the IP address in network byte order to dotted decimal
     * form. Note that we could have used inet_ntoa, but chose not to
     * because inet_ntoa is a Class 3 thread unsafe function that
     * returns a pointer to a static variable (Ch 13, CS:APP).
     */

    
    
    host = ntohl(sockaddr->sin_addr.s_addr);
    a = host >> 24;
    b = (host >> 16) & 0xff;
    c = (host >> 8) & 0xff;
    d = host & 0xff;


    /* Return the formatted log entry string */
    sprintf(logstring, "%s: %d.%d.%d.%d %s %d\n", time_str, a, b, c, d, uri, size);
}


