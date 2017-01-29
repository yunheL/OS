#define _GNU_SOURCE
#include <sched.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "rdtsc.h"

#define BILLION 1000000000L

int main (int argc, char **argv)
{
	if(argc < 2)
	{
		printf("Need number of loop iterations\n");
		return 1;
	}
	//Set the process to stay on CPU[2]
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(2,&mask);
	sched_setaffinity(0, sizeof(mask), &mask);

	int iterations = atoi(argv[1]);
	//int sum = 0, a = 1;

	unsigned long long startRD, endRD;
	struct timespec start, end;
	int  i;
	double diff;
	double rdtscTicks, rdtscFreq_us;
	double max = 0;
	double min = 100;
	
	//determine frequency for rdtsc
	for(i=0;i<100;i++)
	{
		startRD = rdtsc();
		sleep(1);
		endRD = rdtsc();
		rdtscTicks = endRD - startRD;
		
		rdtscFreq_us = (1*1000000)/rdtscTicks;
		if(rdtscFreq_us < min)
		{
			min = rdtscFreq_us;
		}
		if(rdtscFreq_us > max)
		{
			max = rdtscFreq_us;
		}
	}
	printf("Max(us): %lf\n", max);
	printf("Min(us): %lf\n", min);
	return 0;	
	
}
