#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h> 

#define BILLION 1000000000L

int main()
{

	uint64_t diff;
	struct timespec start, end;

	char* s = "helloworldhelloworldhelloworldhelloworldhelloworldhelloworld";
	uint32_t t = 0;
	size_t l = strlen(s);
	int i = 0;

	//a O(n^2) algorithm
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(i = 0; i<strlen(s); i++)
	{
		t += s[i];
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);			
	diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("O(n^2) elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);

	//a O(n) algorithm
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(i = 0; i<l; i++)
	{
		t += s[i];
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);			
	diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("O(n) elapsed time = %llu nanoseconds\n", (long long unsigned int) diff);

	return 0;
}
