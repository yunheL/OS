#include <stdio.h>
#include "rdtsc.h"

int main(int argc, char* argv[])
{
  unsigned long long a,b;

  a = rdtsc();
  sleep(1);
  b = rdtsc();

  printf("a: %llu\n", a);
  printf("b: %llu\n", b);
  printf("b-a: %llu\n", b-a);
  return 0;
}
