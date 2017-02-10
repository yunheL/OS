#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int
main(int argc, char* argv[])
{
	uint32_t a = 1;

	//passed thus printed "here\0"
	assert(a != 2);
	assert(a != 3);
	printf("here0\n");

	//didn't pass and aborted before printing
	//"here1"
	assert(a == 2);
	printf("here1\n");
	assert(a == 3);


	return 0;
}
