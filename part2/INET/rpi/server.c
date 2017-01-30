//credit to http://www.cs.rpi.edu/~moorthy/Courses/os98/Pgms/server.c
//modified by 736 student Brian Guttag and Yunhe Liu
/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/tcp.h>
#define NUMLOOPS 100000
//#define NUMRUNS 3

void error(char *msg)
{
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	int buf_size = atoi(argv[2]);
	int sockfd, newsockfd, portno, clilen;
	int flag = 1;
	int tr = 1;
	char buffer[buf_size];
	struct sockaddr_in serv_addr, cli_addr;
	int n0, n1;
	if (argc < 3) {
		fprintf(stderr,"ERROR, no port and payload LEN provided\n");
		exit(1);
	}
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	if(-1 == setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int)))
		error("ERROR TCP_NODELAY");
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1)
		error("ERROR SOL_SOCKET");
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	if (bind(sockfd, (struct sockaddr *) &serv_addr,
				sizeof(serv_addr)) < 0) 
		error("ERROR on binding");
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
	if (newsockfd < 0) 
		error("ERROR on accept");

//	int k,i;
//	for(k=0; k<NUMRUNS; k++)
//	{
	int i;
	for(i=0; i<NUMLOOPS; i++)
	{
		bzero(buffer,buf_size+1);
		n0 = read(newsockfd,buffer,buf_size);
		//printf("Here is %d th message: %s\n",i+1,buffer);
		n1 = write(newsockfd,"I got your message",18);
		if (n0 < 0) error("ERROR reading from socket");
		if (n1 < 0) error("ERROR writing to socket");
	}
//	}

	if(-1 == close(sockfd))
		error("close0 failed");
	if(-1 == close(newsockfd))
		error("close1 failed");
	return 0; 
}
