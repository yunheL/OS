<<<<<<< HEAD
#include <sys/time.h>
#include "stdlib.h"

int main() {
//	char buffer[30];
	struct timeval tv;
	
	time_t curtime;
	gettimeofday(&tv, NULL); 
	curtime=tv.tv_sec;
	printf(curtime);
=======
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>

int main() {
	struct timeval start, end;	
	
	gettimeofday(&start, NULL);	
	
	sleep(10);
	
	gettimeofday(&end, NULL);

	printf("%ld\n", ((end.tv_sec * 1000000 + end.tv_usec) 
			- (start.tv_sec * 1000000 + start.tv_usec)));

	return 0;
>>>>>>> 0faf1d68fbe74164d155f11aaa7a109ad95972ef
}
