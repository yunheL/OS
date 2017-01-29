#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include "rdtsc.h"

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
	struct timeval start, end;
	int  i;
	gettimeofday(&start, NULL);
	for (i = 0; i < iterations; i++)
	{
		sum = a + sum;
	}
	gettimeofday(&end, NULL);
	long gettimeDiff = (end.tv_sec *1000000 + end.tv_usec) - (start.tv_sec *1000000 + start.tv_usec);
	printf("gettimeofday timing(us): %ld\n",gettimeDiff);

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
	printf("Ticks per second: %lf\n",rdtscTicks);
	double rdtscFreq_us = (1*1000000)/rdtscTicks;
	printf("Frequency of rdtsc in us/tick: %lf\n",rdtscFreq_us);
	double rdtscDiffTime = rdtscDiff * rdtscFreq_us;
	printf("rdtsc timing(us): %lf\n",rdtscDiffTime);

	return 0;	
	
}
