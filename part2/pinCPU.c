#define _GNU_SOURCE
#include <sched.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	cpu_set_t mask;
	CPU_ZERO(&mask);
	CPU_SET(1,&mask);
	sched_setaffinity(0, sizeof(mask), &mask);

	sleep(15);
	return 0;
}
