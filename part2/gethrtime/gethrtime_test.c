#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

hrtime_t gethrtime(void);

int main(int argc, char* argv[])
{
	hrtime_t start = gethrtime();
	sleep(1);
	hrtime_t elapsed = gethrtime() - start; 

	printf("time: %lld\n", elapsed);

	return 0;
}
