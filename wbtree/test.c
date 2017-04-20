#include <stdio.h>
#include <stdlib.h>

inline void
mem_flush(volatile void *p)
{
  asm volatile ("clflush (%0)" :: "r"(p));
  asm volatile ("mfence");
}

int 
main()
{
  int a = 1; 
  mem_flush(&a); 
  printf("hello world\n");
  return 0;
}
