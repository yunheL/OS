#include <stdio.h>
#include <stdint.h> 
#include <stdlib.h> 
#include <time.h> 

#define BILLION 1000000000L

int main(int argc, char *argv[])
{
	uint64_t diff;
	struct timespec start, end;
	int i = 0;
	clock_gettime(CLOCK_MONOTONIC, &start);
	//sleep(3);
	for(i = 0; i < BILLION; i++) {}
	clock_gettime(CLOCK_MONOTONIC, &end);
	
	diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);	
	//sleep(3);
	for(i = 0; i < BILLION; i++) {}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);			

	diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("elapsed process CPU time = %llu nanoseconds\n", (long long unsigned int) diff);

	return 0;	
}
