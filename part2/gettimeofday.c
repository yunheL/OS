#include <sys/time.h>
#include "stdlib.h"

int main() {
//	char buffer[30];
	struct timeval tv;
	
	time_t curtime;
	gettimeofday(&tv, NULL); 
	curtime=tv.tv_sec;
	printf(curtime);
}
