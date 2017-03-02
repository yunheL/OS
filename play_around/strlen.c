#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

/*
 This snapshot of the code is a record of idea
 and haven't be compiled yet. 

 TODO
 compile the code
 add timing mechanism to compare the two different
 algorithms
*/

int main()
{
	char* s = "hello";
	uint32_t t = 0;
	size_t l = strlen(s);
	int i = 0;

	//a O(n^2) algorithm
	for(i = 0; i<strlen(s); i++)
	{
		t += s[i];
	}

	//a O(n) algorithm
	for(i = 0; i<l; i++)
	{
		t += s[i];
	}

	return 0;

}
