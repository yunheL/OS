#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* ran this coupled of times and observed that
even though pid is different each tmie but ppid
is always the same. Use "ps" command and found
out that ppid always refer to the shell in this
case*/
int
main(int argc, char* argv[])
{
	int i;
	pid_t pid = getpid();
	pid_t ppid = getppid();

	for(i=0; i<3; i++)
	{
		printf("pid is %d\n", pid);
		printf("ppid is %d\n", ppid);
	}
	return 0;
}
