#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int print_str(char* msg)
{
	printf("%c\n", *msg);
	printf("%s", msg);

	return 0;
}

int main()
{
	print_str("Success!\n");

	return 0;
}
