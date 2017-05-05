#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include "rdtsc.h"

int main(int argc, char** argv) {
	FILE *fp;
	fp = fopen("ExecutionTimes.txt","r");
	unsigned long time;
	long long int totTime = 0;
	for(int i = 0; i < 1000; i++){
		fscanf(fp,"%lu",&time);
		totTime += time;
	}
	long long int avgTime = totTime/1000;
	printf("Avg Cycles: %lld\n",avgTime);
	fclose(fp);
	return 0;
}
