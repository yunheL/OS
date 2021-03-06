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
#define NUMLOOPS 10000000
//#define NUMLOOPS 5
#define FREQ 0.000313
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

	int sockfd, portno, result, payload_len;
	int flag = 1;
	int tr = 1;
	int n0 = 0;
	int n1 = 0;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	long long int a, b;
	long long int max, min, diff;
	long long int sum = 0;
	double avg;

	payload_len = atoi(argv[3]);
	char buffer[256];
	char buffer1[payload_len];
	if (argc < 4) {
		fprintf(stderr,"usage %s hostname port payload_length\n", argv[0]);
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

	int i,k;
	int read_num = 0;
	double progress = 0;
	double interval = 100/(double)NUMLOOPS;
	int progress_counter = 0;	
	k = 0; // use k to keep track of bytes written and have read
	for(i=0; i<NUMLOOPS; i++)
	{
//		printf("Please enter the message: ");
//		bzero(buffer,256);
//		fgets(buffer,255,stdin);

		bzero(buffer1,payload_len);
		read_num = 0;
		//start timing
		a = rdtsc();
//		n0 = write(sockfd,buffer,strlen(buffer));
		
		for(k=0; k<payload_len; k=k+n0)
		{
			n0 = write(sockfd,payload+k,payload_len-k);

			if (n0 < 0) 
				error("ERROR writing to socket");

//			printf("have written: %d bytes\n", n0);
		}
		b = rdtsc();
		//for throughput end timing after send

		//these two loops cannot be combined to one
		for(k=0; k<payload_len; k=k+n1)
		{
			n1 = read(sockfd,buffer1+k,payload_len-k);

			if (n1 < 0) 
				error("ERROR reading from socket");

//			printf("have read: %d bytes\n", n1);
//			read_num = read_num + n1;
		}
		diff = b-a;

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
		progress = progress + interval;
		if (progress>=5)
		{
			progress_counter = progress_counter+5;
			progress = 0;
			printf("progress: %d percent\n", progress_counter);
		}
		//printf("diff: %llu ticks\n", diff);
	}
//	printf("read_num: %d\n", read_num);
	printf("run %d cycles, length is %zu, first 4 char: %.*s\n", i, sizeof(buffer1), 4,buffer1);
	if(-1 == close(sockfd))
		error("close failed\n");

	avg = ((double)sum)/NUMLOOPS;

	double max_us, min_us, avg_us;
	max_us = max*FREQ;
	min_us = min*FREQ;
	avg_us = avg*FREQ;

	printf("max: %llu ticks, %lf us\n", max, max_us);
	printf("min: %llu ticks, %lf us\n", min, min_us);
	printf("avg: %lf ticks, %lf us\n", avg, avg_us);

	//keep in mind these are not round trip times
	//converting to throughput
	//payload_len indicates the msg size
	double throughput;
	//payload_len have the unit of B, min_us have the unit of us
	//thus the unite for throughput is B/us which has the same value as MB/s
	throughput = payload_len/min_us;
	printf("throughput: %lf (B/us)\n", throughput);
//	}
	return 0;
}
