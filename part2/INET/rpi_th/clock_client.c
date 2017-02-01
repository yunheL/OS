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
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>
#define BILLION 1000000000L
#define NUMLOOPS 100000
//#define NUMRUNS 3

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

	int sockfd, portno, result, n0, n1, payload_len;
	int flag = 1;
	int tr = 1;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	uint64_t diff;
	struct timespec start,end;
	
	uint64_t max, min;
	uint64_t sum = 0;
	double avg;

	char buffer[256];
	char buffer1[256];
	if (argc < 4) {
		fprintf(stderr,"usage %s hostname port payload_length\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	payload_len = atoi(argv[3]);
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

	char pattern[4] = {'a', 'b', 'c', 'd'};
	char payload[payload_len];
	int j = 0;
	for (j=0; j<payload_len; j=j+4)
	{
		memcpy(&payload[j], pattern, 4);
	}

//	int k,i;
//	for(k = 0; k<NUMRUNS; k++)
//	{
	
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
	error("ERROR connecting");

	int i;	
	for(i=0; i<NUMLOOPS; i++)
	{
//		printf("Please enter the message: ");
//		bzero(buffer,256);
//		fgets(buffer,255,stdin);

		bzero(buffer1,256);
		//start timing
		clock_gettime(CLOCK_MONOTONIC, &start);
//		n0 = write(sockfd,buffer,strlen(buffer));
		n0 = write(sockfd,payload,payload_len);
		n1 = read(sockfd,buffer1,255);
		clock_gettime(CLOCK_MONOTONIC, &end);
		diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
		//end timing

		if (n0 < 0) 
			error("ERROR writing to socket");
		if (n1 < 0) 
			error("ERROR reading from socket");

		if(i == 0)
		{
			max = diff;
			min = diff;
		}
		else{
			if(diff > max)
				max = diff;
			if(diff < min)
				min = diff;
		}

		sum += diff;
		//printf("diff: %llu ticks\n", diff);
	}

	printf("%s\n",buffer1);
	if(-1 == close(sockfd))
		error("close failed\n");

	avg = ((double)sum)/NUMLOOPS;
	printf("max: %" PRIu64 " ns\n", max);
	printf("min: %" PRIu64 " ns\n", min);
	printf("avg: %lf ns\n", avg);
//	}
	return 0;
}
