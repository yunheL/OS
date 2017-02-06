#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define LEN 128

struct point{
	int x;
	int y;
};

int main(void)
{
	//struct test
	struct point a;
	a.x = 1;
	a.y = 1;
	struct point b;
	b.x = a.x * 2;
	b.y = a.y * 2;

	printf("b[%d, %d]\n", b.x, b.y);

	//pointer test
	int* k;
	int j = 5;

	k = &j;

	//printf("k: %p",k);
	printf("*k: %d\n",*k);

	char pattern[4]={'a', 'b', 'c', 'd'};
	printf("pattern0: %c\n",pattern[0]);
	printf("pattern1: %c\n",pattern[1]);
	printf("pattern2: %c\n",pattern[2]);
	printf("pattern3: %c\n",pattern[3]);

	char payload[LEN];
	int i;
	for(i=0; i<LEN; i=i+4)
	{
		memcpy(&payload[i], pattern, 4);
	}
	
	for(i=0; i<LEN; i++)
	{
		printf("payload%d: %c\n", i, payload[i]);
	}	
	return 0;
}
