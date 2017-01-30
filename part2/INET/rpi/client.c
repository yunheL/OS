//credit to http://www.unixguide.net/network/socketfaq/2.16.shtml
//modified by students from cs736 Brian Guttag and Yunhe Liu on 1/29/2017
#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <unistd.h>
#include <netinet/tcp.h>
#include "rdtsc.h"

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(3,&mask);
	sched_setaffinity(0, sizeof(mask), &mask);

	int sockfd, portno, result, n0, n1;
	int flag = 1;
	int tr = 1;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	long long int a, b;
	char buffer[256];
	char buffer1[256];
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");
	result = setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&flag, sizeof(int));
	if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1)
		error("ERROR SOL_SOCKET");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, 
			(char *)&serv_addr.sin_addr.s_addr,
			server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
		error("ERROR connecting");

	int i = 0;
	for(i=0; i<5; i++)
	{
		printf("Please enter the message: ");
		bzero(buffer,256);
		bzero(buffer1,256);
		fgets(buffer,255,stdin);

		//start timing
		a = rdtsc();
		n0 = write(sockfd,buffer,strlen(buffer));
		n1 = read(sockfd,buffer1,255);
		b = rdtsc();
		//end timing

		if (n0 < 0) 
			error("ERROR writing to socket");
		if (n1 < 0) 
			error("ERROR reading from socket");
		printf("%s\n",buffer1);
	}

	if(-1 == close(sockfd))
		error("close failed\n");

	printf("a: %llu\n", a);
	printf("b: %llu\n", b);
	printf("b-a: %llu\n", b-a);
	return 0;
}
