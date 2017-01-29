#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define BILLION 1000000000L
#define NUM_LOOP 50

int main(int argc, char *argv[])
{
	//pin process to a specfic CPU
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(2,&mask);
	sched_setaffinity(0, sizeof(mask), &mask);

	uint64_t diff;
	struct timespec start, end, test;
	int i = 0;
	double timing = 0.0;

	int j = 0; //outer loop
	double max, min, avg;
	double sum = 0;

	double progress = 0;
	double interval = (double) (100/(double)NUM_LOOP);

	printf("interval: %lf percent each loop\n", interval);

	for(j=0; j<NUM_LOOP; j++)
	{
			
		clock_gettime(CLOCK_MONOTONIC, &start);
		for(i = 0; i < BILLION; i++)
			clock_gettime(CLOCK_MONOTONIC, &test);

		clock_gettime(CLOCK_MONOTONIC, &end);		

			diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
		//printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);
		
		timing = (double)diff / (double)BILLION;
		if(timing < 15 || timing > 16)
			printf("WARNING!!!!!!!!!!!\n");

		if(j == 0)
		{
			max = timing;
			min = timing;
		}
		else{
			if(timing > max)
				max = timing;
			if(timing < min)
				min = timing;
		}
		sum += timing;
		
		progress = (j+1) * interval;
		printf("Progress: %lf percent. Timing: %lf ns\n", progress, timing);
	}

	avg = sum/NUM_LOOP;

	printf("max: %lf ns\n", max);
	printf("min: %lf ns\n", min);
	printf("avg: %lf ns\n", avg);

	return 0;

}
