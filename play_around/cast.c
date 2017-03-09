#include <stdlib.h>
#include <stdio.h>

int main()
{
	//cast int into char
	int C = 67;
	char test_char = (char)C;

	//cast char into int
	char big_c = 'C';
	int test_int = (int)big_c;

	printf("test_char is %c\n", test_char);
	printf("test_int is %d\n", test_int);

	return 0;
}
