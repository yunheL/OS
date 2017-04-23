#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define MAXKEY 7
#define MAXSLOT (MAXKEY+1)
#define MAXPOINTER MAXSLOT
#define MAXRECORD MAXSLOT

#define SIZE(n) ((n)->slot_array[0] & 0x7f)
#define LASTLV(n) ((n)->slot_array[0] & 0x80)

void
mem_flush(volatile void *p)
{
  asm volatile ("clflush (%0)" :: "r"(p));
  asm volatile ("mfence");
}

typedef uint64_t KeyType;
typedef uint64_t RecordType;

typedef struct NonLeaf{
  uint8_t slot_array[MAXSLOT];
  KeyType key_array[MAXKEY];
  void* pointer_array[MAXPOINTER];
} NonLeaf;

typedef struct Leaf{
  uint8_t slot_array[MAXSLOT];
  struct Leaf* next;
  KeyType key_array[MAXKEY];
  RecordType record_array[MAXRECORD];
} Leaf;


