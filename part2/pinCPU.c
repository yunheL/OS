#include <sched.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(0,&mask);
	sched_setaffinity(0, sizeof(mask), &mask);

	sleep(30);
	return 0;
}
