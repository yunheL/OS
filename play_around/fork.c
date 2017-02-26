#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
	int pid = fork();
	
	if(pid == 0)
	{
		printf("child: pid is %d\n", pid);
	}

	if(pid > 0)
	{
		printf("parent: pid is %d\n", pid);
	}

	return 0;
}
