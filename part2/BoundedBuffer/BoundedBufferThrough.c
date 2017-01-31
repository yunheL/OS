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
#define ITERATIONS 10
#define T_I 10
pthread_mutex_t lockWrite;
pthread_mutex_t lockRead
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
	int i,j,k,m;
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
	double through, data;
	
	fd = open("membufferfile",O_RDWR);
	size = sizeArray[index]; 
	printf("Size: %d\n",size);
	message = (char *) malloc(size);	//create an array of size (bytes)
	if(message == NULL) {perror("memory"); exit(EXIT_FAILURE); }
	makeMessage(&message,size);
	buff = mmap(NULL,size,PROT_READ |PROT_WRITE,MAP_SHARED,fd,0);
	if(buff == MAP_FAILED) {perror("mmap"); exit(EXIT_FAILURE);}
	if(pthread_mutex_init(&lockWrite, NULL) != 0) {perror("mutex Write creation"); exit(EXIT_FAILURE);}
	if(pthread_mutex_init(&lockRead, NULL) != 0 ) {perror("mutex Read creation"); exit(EXIT_FAILURE);}
	for(i=0;i<ITERATIONS;i++){
		cpid = fork();
		if(cpid == 1) {perror("fork"); exit(EXIT_FAILURE); }
	
		if(cpid == 0) { //child
			
			
			while(m < T_I){
				pthread_mutex_lock(&lockRead);
				j =0;
				while(j < size){
					read(fd,&buf,1);
					j++;
				}
				pthread_mutex_unlock(&lockWrite);
				m++;
			}
			exit(EXIT_SUCCESS);
		}
		else { //parent
			start = rdtsc();
			k = 0;
			while(k < T_I){
				pthread_mutex_lock(&lockWrite);
				write(fd,message,size);
				pthread_mutex_unlock(&lockRead);
				k++;
			}
			
			end = rdtsc();
			wait(NULL);
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
	data = size*T_I;
	through = data/diffTime;
	printf("Throughput(B/us): %lf\n",through);


	return 0;
	
}
