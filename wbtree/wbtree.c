#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include "wbtree.h"

int
insert(void* vnode, _Bool isLeaf, KeyType newkey, RecordType newval, KeyType* upkey, void** upchild)
{
  if(!isLeaf)
  {
    NonLeaf* node = (NonLeaf*)vnode;
    uint8_t n = SIZE(node);
    uint8_t pos_slot = -1;
    uint8_t pos_key = -1;
    //binary search
    {
      int l = 0, r = n;
      while (l < r) {
        int m = l + (r - l) / 2;
        if (node->key_array[node->slot_array[m]] > newkey) {
          r = m;
        } else {
          l = m + 1;
        }
      }
      assert(l == r);
      assert(0 <= l && l <= n);
      pos_slot = l;
    }
    if (pos_slot < n) {
      pos_key = node->slot_array[pos_slot];  // [..., key]
    } else {
      pos_key = MAXKEY;                 // [..., +inf]
    }
    KeyType splitkey;
    void* splitchild;
    insert(node->pointer_array[pos_key], LASTLV(node), newkey, newval, &splitkey, &splitchild);
    if (splitchild == NULL) {
      return 0;
    }
    if (n < MAXKEY) {  // insert key from children split
      uint8_t empty = -1;
      {
        _Bool used[MAXKEY] = {0};
        for (int i = 0; i < n; i++) { used[node->slot_array[i]] = 1; }
        for (empty = 0; empty < MAXKEY; empty++) { if(!used[empty]) break; }
        assert(empty < MAXKEY);
      }
      node->key_array[empty] = splitkey;
      node->pointer_array[empty] = splitchild;
      mem_flush(&(node->key_array[empty]));
      mem_flush(&(node->pointer_array[empty]));
      uint8_t tempslot[8];
      for (int i = n; i > pos_slot; i--) { tempslot[i] = node->slot_array[i-1]; }
      tempslot[pos_slot] = empty;
      for (int i = pos_slot-1; i >= 0; i--) { tempslot[i] = node->slot_array[i]; }
      tempslot[0]++;
      *(uint64_t*)(node->slot_array) = *(uint64_t*)tempslot;
      mem_flush(node->slot_array);
    } else {
      // split: keep LASTLV
    }
  } else {
    // leaf
  }
  
  return 0;
}

int 
main()
{
  // build empty tree: root -> NL -> Leaf, with all sizes = 0
  return 0;
}
