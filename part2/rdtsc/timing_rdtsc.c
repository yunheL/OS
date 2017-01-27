#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include "rdtsc.h"

#define BILLION 1000000000

int main(int argc, char *argv[])
{
	unsigned long long a,b,c;
	int i = 0;
	int j = 0;
	double diff = 0.0; 

	for(j=0; j<10; j++)
	{	
		a = rdtsc();
		for(i=0; i<BILLION; i++)
			c = rdtsc();

		b = rdtsc();
		diff = ((double)(b-a))/BILLION;
		if(diff < 21 || diff > 22)
			printf("WARNING!!!!!!!!!!!\n");
			

	
		printf("a: %llu\n", a);
		printf("b: %llu\n", a);
		printf("b-a: %llu\n", b-a);
		printf("timimg: %lf\n", diff);
	}
}
