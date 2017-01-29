#define _GNU_SOURCE
#include <sched.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/wait.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "rdtsc.h"

int main(int argc, char *argv[])
{
	int pfd[2]; //file descriptor(2) pointing to pipe inode, pfd[0] for reading, pfd[1] for writing
	pid_t cpid;
	char buf;

	//set up file to write to
	int fd;
	fd = open("testPipe.txt", O_CREAT | O_WRONLY, 0600);
	if(fd == -1){ printf("Failed to create and open the file\n"); exit(1); }	

	assert(argc == 2);
	if(pipe(pfd) == -1) { perror("pipe"); exit(EXIT_FAILURE); }
	
	cpid = fork();
	if (cpid == -1) { perror("fork"); exit(EXIT_FAILURE); }
	
	if(cpid == 0) {	//child process
		close(pfd[1]); //child doesn't write
		while(read(pfd[0],&buf, 1) > 0)
			write(fd, &buf, 1);
		write(fd, "\n", 1);
		close(pfd[0]);
		close(fd);
		_exit(EXIT_SUCCESS);
	} else {	//parent process
		close(pfd[0]);	//parent doesn't read
		printf("the line is: ");
		printf("%s",argv[1]);
		printf("\n");
		write(pfd[1], argv[1], strlen(argv[1]));
		close(pfd[1]);
		wait(NULL);	//wait for child to finish
		exit(EXIT_SUCCESS);
	}
}
