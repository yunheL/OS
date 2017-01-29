#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "rdtsc.h"
#include <stdint.h>

#define BILLION 1000000000L

int main (int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Need number of loop iterations\n");
		return 1;
	}
	int iterations = atoi(argv[1]);
	int sum = 0, a = 1;
	unsigned long long startRD, endRD;
	struct timespec start, end;
	int  i;
	double diff;


	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
	for(i = 0; i < iterations; i++)
	{
		sum = a + sum;	
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);
	diff = (end.tv_sec - start.tv_sec)*BILLION + (end.tv_nsec - start.tv_nsec); 
	printf("elapsed process CPU time = %lf microseconds\n", (diff/1000));

	sum = 0;
	startRD = rdtsc();
	for (i = 0; i < iterations; i++)
	{
		sum = a + sum;
	}
	endRD = rdtsc();
	double rdtscDiff = endRD - startRD;
	
	//determine frequency for rdtsc
	startRD = rdtsc();
	sleep(1);
	endRD = rdtsc();
	double rdtscTicks = endRD - startRD;
	
	double rdtscFreq_us = (1*1000000)/rdtscTicks;
	
	double rdtscDiffTime = rdtscDiff * rdtscFreq_us;
	printf("rdtsc timing(us): %lf\n",rdtscDiffTime);
	
	printf("difference between the two(us): %lf\n",((diff/1000)-rdtscDiffTime));
	return 0;	
	
}
