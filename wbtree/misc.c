#include <stdio.h>
void div(int dividee , int divisor, int* quotient, int* remainder){
  *quotient = dividee / divisor;
  *remainder = dividee % divisor;;
}
int main() {
  int q, r;
  div(12, 5, &q, &r);
  printf("%d / %d = %d ... %d\n", 12, 5, q, r);
  return 0;
}
