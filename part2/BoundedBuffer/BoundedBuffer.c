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
#include <semaphore.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <sys/mman.h>
#include "rdtsc.h"
#define FREQ 0.000313
#define ITERATIONS 1
pthread_mutex_t lock;
pthread_t tid[2];

void makeMessage(char **mess, int size) {
	int i;
	char carray[4]= "abcd";
	for(i=0;i<size;i++)
	{
		(*mess)[i] = carray[(i%4)];
	}
}

int main(int argc, char *argv[])
{
	assert(argc == 2);
	int size;
	int i,j;
	int index = atoi(argv[1]);
	int sizeArray[10] = {4,16,64,256,1024,4096,16384,65536,262144,524288};
	char *message;
	pid_t cpid;
	int fd;
	char buf;
	char *buff;
	unsigned long long start, end;
	unsigned long long diffTicks;
	unsigned long long diffMin = 1000000000000;
	double diffTime;
	
	fd = open("membufferfile",O_RDWR);
	size = sizeArray[index]; 
	printf("Size: %d\n",size);
	message = (char *) malloc(size);	//create an array of size (bytes)
	if(message == NULL) {perror("memory"); exit(EXIT_FAILURE); }
	makeMessage(&message,size);
	buff = mmap(NULL,size,PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
	if(buff == MAP_FAILED) {perror("mmap"); exit(EXIT_FAILURE);}
	if(pthread_mutex_init(&lock, NULL) != 0) {perror("mutex creation"); exit(EXIT_FAILURE);}
	for(i=0;i<ITERATIONS;i++){
		cpid = fork();
		if(cpid == 1) {perror("fork"); exit(EXIT_FAILURE); }
	
		if(cpid == 0) { //child
			pthread_mutex_lock(&lock);
			j =0;
			while(j < size){
				read(fd,&buf,1);
				j++;
			}
			//while(read(fd,&buf, 1) > 0){}
			pthread_mutex_unlock(&lock);
			exit(EXIT_SUCCESS);
		}
		else { //parent
			start = rdtsc();
			pthread_mutex_lock(&lock);
			write(fd,message,size);
			pthread_mutex_unlock(&lock);
			wait(NULL);
			end = rdtsc();

			diffTicks = end - start;
		}
		if(diffTicks < diffMin){
			diffMin = diffTicks;
		}
	}
	close(fd);
	diffTime = diffMin*FREQ;
	printf("Ticks: %llu\n", diffMin);
	printf("Time(us): %lf\n",diffTime);
	


	return 0;
	
}
