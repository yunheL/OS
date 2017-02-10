#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int
main(int argc, char* argv[])
{
	pid_t pid = getpid();
	printf("pid is %d\n", pid);
	return 0;
}
