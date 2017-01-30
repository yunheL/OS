#define _GNU_SOURCE
#include <sched.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rdtsc.h"

#define FREQ 0.000313
#define ITERATIONS 100000

void makeMessage(char **mess, int size);
unsigned long long createPipe(int size, char *message);

int main(int argc, char *argv[])
{	
	assert(argc == 2);
	int index = atoi(argv[1]);
	int size;
	double diffTime;
	unsigned long long diff;
	int i;
	unsigned long long diffMin = 1000000000;
	char *message;
	//select sizing
	int sizeArray[10] = {4,16,64,256,1000,4000,16000,64000,256000,512000};
	size = sizeArray[index]; 
	printf("Size: %d\n",size);
	//set up message
	message = (char *) malloc(size);	//create an array of size (bytes)
	if(message == NULL) {perror("memory"); exit(EXIT_FAILURE); }
	makeMessage(&message,size);
	
	for(i = 0; i < ITERATIONS; i++){
		diff = createPipe(size, message);
		if(diff < diffMin){
			diffMin = diff;
		}	
	}
	diffTime = diffMin*FREQ;
	printf("Ticks: %llu\n", diffMin);
	printf("Time(us): %lf\n",diffTime);
	free(message);
	return 0;
}

unsigned long long createPipe(int size, char *message)
{
	int pfd[2]; //file descriptor(2) pointing to pipe inode, pfd[0] for reading, pfd[1] for writing
	pid_t cpid;
	char buf;
	char buf2;
	cpu_set_t mask;
	unsigned long long start, end;
	unsigned long long diffTicks;

	
	//set up file to write to
	//int fd;
	//fd = open("testPipe.txt", O_CREAT | O_WRONLY, 0600);
	//if(fd == -1){ perror("file"); exit(EXIT_FAILURE); }

	if(pipe(pfd) == -1) { perror("pipe"); exit(EXIT_FAILURE); }

	cpid = fork();
	if (cpid == -1) { perror("fork"); exit(EXIT_FAILURE); }
	
	if(cpid == 0) {	//child process
		//close(pfd[1]); //child doesn't write
		while(read(pfd[0],&buf, 1) > 0)
		//	write(fd, &buf, 1);
		//write(fd, "\n", 1);
		close(pfd[0]);
		//close(fd);
		_exit(EXIT_SUCCESS);
	} else {	//parent process
		//keep the parent (the place your timing) on the same CPU
		CPU_ZERO(&mask);
		CPU_SET(2,&mask);
		sched_setaffinity(cpid, sizeof(mask), &mask);
		
		close(pfd[0]);	//parent doesn't read
		//argum = (long)fcntl(pfd[1],F_GETFL,O_NONBLOCK);
		int ret = fcntl(pfd[1], F_SETPIPE_SZ, 512000);
		if(ret<0){ perror("set pipe size failed"); }
		long pipe_size = (long)fcntl(pfd[1], F_GETPIPE_SZ);
		if(pipe_size == -1) { perror("get pipe size failed"); }
	
		start = rdtsc();
		for(j=0; j<THR_ITER; j++){
			write(pfd[1], message, size);
		}
		
		close(pfd[1]);
		wait(NULL);	//wait for child to finish
		end = rdtsc();

		diffTicks = end - start;
		//close(fd);
		return diffTicks;
	}
}

void makeMessage(char **mess, int size) {
	int i;
	char carray[4]= "abcd";
	for(i=0;i<size;i++)
	{
		(*mess)[i] = carray[(i%4)];
	}
}

