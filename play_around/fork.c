#include <stdio.h>
#include <stdlib.h>

int main()
{
	int pid = fork();
	printf("pid is %d", pid);

	return 0;
}
