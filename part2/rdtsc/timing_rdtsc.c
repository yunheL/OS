#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "rdtsc.h"
#include <unistd.h>

#define BILLION 1000000000L
#define NUM_LOOP 100L

int main(int argc, char *argv[])
{
	//pin process to a specfic CPU
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(3,&mask);
	sched_setaffinity(0, sizeof(mask), &mask);

//	double a,b,c;
	unsigned long long a,b,c;
	int i = 0; //inner loop
	int j = 0; //outer loop
	double diff = 0.0; 

	double max, min, avg;
	double sum = 0;

	double progress = 0;
	double interval = (double) (100/(double)NUM_LOOP);
	
	printf("interval: %lf percent\n", interval);

	for(j=0; j<NUM_LOOP; j++)
	{	
		a = rdtsc();
		for(i=0; i<BILLION; i++)
			c = rdtsc();

		b = rdtsc();
		diff = (double)(b-a)/BILLION;
		if(diff < 21 || diff > 22)
			printf("WARNING!!!!!!!!!!!\n");
			
		//initialize max, min value in the first loop
		if(j == 0)
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


		progress = (j+1)*interval;
		printf("Progress: %lf percent\n", progress);
/*
		printf("a: %llu\n", a);
		printf("b: %llu\n", a);
		printf("b-a: %llu\n", b-a);
		printf("timimg: %lf\n", diff);
*/

	}//end of timing loop
		
	avg = sum/NUM_LOOP;

	printf("max: %lf ticks\n", max);
	printf("min: %lf ticks\n", min);
	printf("avg: %lf ticks\n", avg);

	return 0;
}
