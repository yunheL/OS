#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <assert.h>

#ifdef TRAD
#define MAXKEY 7
#define SLOTWIDTH 3
#else
#define MAXKEY 15
#define SLOTWIDTH 4
#endif

/* EVAL */
#define BILLION 1000000000L
#define NUMINSERT 100000
#define NUMDELETE NUMINSERT/2
#define EVALLOOP 2

typedef struct timeTuple{
  uint64_t insertTime; 
  uint64_t deleteTime;
} timeTuple;

#define MAXSLOT (MAXKEY+1)
static_assert(MAXKEY <= (1 << SLOTWIDTH), "width not enough");
static_assert(SLOTWIDTH * MAXSLOT <= 64, "not fit into 64 bits");
#define MAXPOINTER MAXSLOT
#define MAXRECORD MAXKEY

#define MAXSTACK 10
#define MAXATOM 2
#define PENDATOM(LV, RV) do { \
  assert(atom_n < MAXATOM);   \
  atom_addr[atom_n] = (uint64_t*)&(LV);  \
  atom_newv[atom_n] = (RV);   \
  atom_n++;                   \
} while (0)

#define MASK ((uint64_t)((1<<SLOTWIDTH)-1))
#define SIZE(n) ((n)->slot_array & MASK)
#define NTH(sa, n) (((sa) >> ((n)*SLOTWIDTH)) & MASK)
#define SNTH(sa, n, v) do{ (sa) = (((sa) & ~(MASK << ((n)*SLOTWIDTH))) | (((uint64_t)(v)) << ((n)*SLOTWIDTH))); } while (0)

static inline void
mem_flush(volatile void *p)
{
  asm volatile ("clflush (%0)" :: "r"(p));
  asm volatile ("mfence");
}
void clflush(volatile void* p) {
  asm volatile ("clflush (%0)" :: "r"(p));
}
void mfence() {
  asm volatile ("mfence");
}

typedef uint64_t KeyType;
typedef uint64_t RecordType;

typedef struct NonLeaf{
  uint64_t slot_array;
  KeyType key_array[MAXKEY];
  void* pointer_array[MAXPOINTER];
} NonLeaf;

typedef struct Leaf{
  uint64_t slot_array;
  KeyType key_array[MAXKEY];
  RecordType record_array[MAXRECORD];
  struct Leaf* next;
} Leaf;

typedef struct {
  NonLeaf* root;
  int height;
} Tree;

NonLeaf* new_NonLeaf() {
  return malloc(sizeof(NonLeaf));
}
Leaf* new_Leaf() {
  return malloc(sizeof(Leaf));
}
