#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

//inline
void
mem_flush(volatile void *p)
{
  asm volatile ("clflush (%0)" :: "r"(p));
  asm volatile ("mfence");
}

typedef uint64_t KeyType;
typedef uint64_t RecordType;

#define MAXKEY 7
#define SIZE(n) ((n)->slot_array[0] & 0x7f)
#define LASTLV(n) ((n)->slot_array[0] & 0x80)

typedef struct NonLeaf{
  uint8_t slot_array[8];
  KeyType key_array[7];
  //cast it when pointing to leaf and non_leaf node
  void* pointer_array[8];
} NonLeaf;

typedef struct Leaf{
  uint8_t slot_array[8];
  struct Leaf* next;
  KeyType key_array[7];
  RecordType record_array[8];
} Leaf;


