//reference: <http://stackoverflow.com/questions/2702156/
//what-is-the-difference-between-the-and-the-operators-in-c-programming>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int
main(int argc, char* argv[])
{
	int i;
	printf("i address is %p\n", &i);
	int* p;
	printf("p initially is %p\n", p);
	p = &i;
	printf("p now is %p\n", p);
	*p = 3;
	printf("now i is %d\n", i);
	
	return 0;
}
