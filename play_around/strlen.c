#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <time.h> 

#define BILLION 1000000000L

//TODO looks like diff0 and diff1 is not correct. The value is too large.
//TODO tried to use a single char for string s, the difference didn't
//	decrease at all. Not correct.

int main()
{

	uint64_t diff0;//timing of O(n^2)
	uint64_t diff1;//timing of O(n)
	uint64_t diff;//timing difference between O(n^2) and O(n)
	struct timespec start, end;

//	char* s = "helloworldhelloworldhelloworldhelloworldhelloworldhelloworld";
	char* s = "h";
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
	diff0 = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("O(n^2) elapsed time = %llu nanoseconds\n", (long long unsigned int) diff0);

	//a O(n) algorithm
	clock_gettime(CLOCK_MONOTONIC, &start);
	for(i = 0; i<l; i++)
	{
		t += s[i];
	}
	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end);			
	diff1 = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
	printf("O(n) elapsed time = %llu nanoseconds\n", (long long unsigned int) diff1);

	diff = diff0-diff1;	
	printf("diff = %llu nanoseconds\n", (long long unsigned int) diff);

	return 0;
}
