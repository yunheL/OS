#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#define BILLION 1000000000L

int main(int argc, char *argv[])
{
	uint64_t diff;
	struct timespec start, end, test;
	int i = 0;
	double timing = 0.0;

	clock_gettime(CLOCK_MONOTONIC, &start);
	for(i = 0; i < BILLION; i++)
		clock_gettime(CLOCK_MONOTONIC, &test);

	clock_gettime(CLOCK_MONOTONIC, &end);		

		diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);
	
	timing = (double)diff / (double)BILLION;
	printf("timing: %lf\n", timing);

	return 0;
}
