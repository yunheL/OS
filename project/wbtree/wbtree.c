#include "wbtree.h"

int addr2sym(void* p) {
  static void* buf[100] = {NULL};
  static int n = 1;
  for (int i = 0; i < n; i++) {
    if (buf[i] == p) {
      return i;
    }
  }
  buf[n++] = p;
  return n-1;
}

int locate_last(KeyType* key_array, uint64_t slot_array, uint8_t n, KeyType newkey) {
  int l = 1, r = n+1;
  while (l < r) {
    int m = l + (r - l) / 2;
    if (key_array[NTH(slot_array,m)] > newkey) {
      r = m;
    } else {
      l = m + 1;
    }
  }
  assert(l == r);
  assert(0 <= l-1 && l-1 <= n);
  return l-1;
}
int locate_first(KeyType* key_array, uint64_t slot_array, uint8_t n, KeyType newkey) {
  int l = 1, r = n+1;
  while (l < r) {
    int m = l + (r - l) / 2;
    if (key_array[NTH(slot_array,m)] >= newkey) {
      r = m;
    } else {
      l = m + 1;
    }
  }
  assert(l == r);
  assert(1 <= l && l <= n+1);
  return l;
}
uint8_t locate_empty(uint64_t slot_array, uint8_t n) {
  uint8_t empty = -1;
  _Bool used[MAXKEY] = {0};
  for (int i = 1; i <= n; i++) { used[NTH(slot_array,i)] = 1; }
  for (empty = 0; empty < MAXKEY; empty++) { if(!used[empty]) break; }
  assert(empty < MAXKEY);
  return empty;
}
void update_slot_with_atom(uint64_t* slot_array, uint8_t n, int pos_slot, uint8_t empty, uint64_t** atom_addr, uint64_t* atom_newv, int* atom_n) {
  uint64_t tempslot = 0;
  /*
  for (int i = n+1; i > pos_slot; i--) { SNTH(tempslot, i, NTH(*slot_array,i-1)); }
  SNTH(tempslot, pos_slot, empty);
  for (int i = pos_slot-1; i >= 0; i--) { SNTH(tempslot, i, NTH(*slot_array,i)); }
  SNTH(tempslot, 0, NTH(tempslot, 0)+1);
  */
  tempslot = (*slot_array & ~((1ll << (pos_slot*SLOTWIDTH)) - 1)) << SLOTWIDTH;
  tempslot = tempslot | (((uint64_t)empty) << (pos_slot*SLOTWIDTH));
  tempslot = tempslot | (*slot_array & ((1ll << (pos_slot*SLOTWIDTH)) - 1));
  tempslot++;
  // LOG BEGIN: TODO FIXME
  *slot_array = tempslot;
  mem_flush(slot_array);
  if (atom_n != NULL) {
    for (int i = 0; i < *atom_n; i++) {
      *(atom_addr[i]) = atom_newv[i];
      mem_flush(atom_addr[i]);
    }
    *atom_n = 0;
  }
  // LOG END
}
void update_slot(uint64_t* slot_array, uint8_t n, int pos_slot, uint8_t empty) {
  update_slot_with_atom(slot_array, n, pos_slot, empty, NULL, NULL, NULL);
}

Tree* init_empty_tree() {
  // build empty tree: root -> NL -> Leaf, with all sizes = 0
  NonLeaf* root = new_NonLeaf();
  root->slot_array = 0;
  root->pointer_array[MAXKEY] = ({
    Leaf* leaf0 = new_Leaf();
    leaf0->slot_array = 0;
    leaf0->next = NULL;
    leaf0;
  });
  Tree* t = malloc(sizeof(Tree));
  t->root = root;
  t->height = 2;
  return t;
}

void insert_ng(Tree* tree, KeyType newkey, RecordType newval) {
  static NonLeaf* stack[MAXSTACK];  // on bss
  static uint8_t stack_pos_slot[MAXSTACK];  // on bss
//  static uint8_t stack_pos_key[MAXSTACK];  // on bss
  int top = 0;                      // not static, 0 every time
  uint64_t* atom_addr[MAXATOM];
  uint64_t atom_newv[MAXATOM];
  int atom_n = 0;
  NonLeaf* node = tree->root;
  for (int i = 0; i < tree->height - 1; i++) {
    stack[top++] = node;
    uint8_t n = SIZE(node);
    uint8_t pos_slot = locate_last(node->key_array, node->slot_array, n, newkey);
    uint8_t pos_key = -1;
    if (pos_slot >= 1) {
      pos_key = NTH(node->slot_array,pos_slot);  // [key, ...]
    } else {
      pos_key = MAXKEY;                 // [LB, ...]
    }
    stack_pos_slot[top-1] = pos_slot;
//    stack_pos_key[top-1] = pos_key;
    node = node->pointer_array[pos_key];
  }
  Leaf* leaf = (Leaf*)node;
  KeyType upkey;
  void* upchild = NULL;
  { // LEAF BEGIN
    uint8_t n = SIZE(leaf);
    uint8_t pos_slot = locate_last(leaf->key_array, leaf->slot_array, n, newkey);
    assert(n < MAXKEY);
    {  // insert key directly
      uint8_t empty = locate_empty(leaf->slot_array, n);
      leaf->key_array[empty] = newkey;
      leaf->record_array[empty] = newval;
      mem_flush(&(leaf->key_array[empty]));
      mem_flush(&(leaf->record_array[empty]));
      update_slot(&(leaf->slot_array), n, pos_slot+1, empty);
      n++;
    }
    if (n == MAXKEY) {
      // split
      Leaf* leaf2 = new_Leaf();
      leaf2->slot_array = 0;
      uint8_t n2 = n/2;
      uint8_t n1 = n - n2;
      for (int i = 1; i <= n2; i++) {
        // [(n1 + (i-1))+1] == [n1+i]
        leaf2->key_array[i-1] = leaf->key_array[NTH(leaf->slot_array,n1+i)];
        leaf2->record_array[i-1] = leaf->record_array[NTH(leaf->slot_array,n1+i)];
        clflush(&(leaf2->key_array[i-1]));
        clflush(&(leaf2->record_array[i-1]));
        SNTH(leaf2->slot_array, i,  i-1);
      }
      SNTH(leaf2->slot_array, 0, n2);
      clflush(&(leaf2->slot_array));
      leaf2->next = leaf->next;
      clflush(&(leaf->next));
      mfence();
      // ATOM 1 PENDING
      assert(atom_n == 0);
      PENDATOM(leaf->next, (uint64_t)leaf2);
      PENDATOM(leaf->slot_array, (leaf->slot_array & ~MASK)|n1);
      upkey = leaf->key_array[NTH(leaf->slot_array,n1)];
      upchild = leaf2;
    }
  } // LEAF END
  while (upchild != NULL) {
    if (--top < 0) {
      break;
    }
    node = stack[top];
    uint8_t n = SIZE(node);
    uint8_t pos_slot = stack_pos_slot[top];
//    uint8_t pos_key = stack_pos_key[top];
    assert(n < MAXKEY);
    {  // insert key from children split
      uint8_t empty = locate_empty(node->slot_array, n);
      node->key_array[empty] = upkey;
      node->pointer_array[empty] = upchild;
      mem_flush(&(node->key_array[empty]));
      mem_flush(&(node->pointer_array[empty]));
      // ATOM 1/2 BEGIN
      update_slot_with_atom(&(node->slot_array), n, pos_slot+1, empty, atom_addr, atom_newv, &atom_n);
      assert(atom_n == 0);
      // ATOM 1/2 END
      upchild = NULL;
      n++;
    }
    if (n == MAXKEY) {
      NonLeaf* node2 = new_NonLeaf();
      node2->slot_array = 0;
      uint8_t n1 = n/2;
      uint8_t n2 = n-1 - n1;
      for (uint8_t i = 1; i <= n2; i++) {
        // [(n1+1 + (i-1))+1] == [n1+1+i]
        node2->key_array[i-1] = node->key_array[NTH(node->slot_array,n1+1+i)];
        node2->pointer_array[i-1] = node->pointer_array[NTH(node->slot_array,n1+1+i)];
        clflush(&(node2->key_array[i-1]));
        clflush(&(node2->pointer_array[i-1]));
        SNTH(node2->slot_array, i, i-1);
      }
      SNTH(node2->slot_array, 0, n2);
      clflush(&(node2->slot_array));
      node2->pointer_array[MAXKEY] = node->pointer_array[NTH(node->slot_array,n1+1)];
      clflush(&(node2->pointer_array[MAXKEY]));
      mfence();
      // ATOM 2 PENDING
      assert(atom_n == 0);
      PENDATOM(node->slot_array, (node->slot_array & ~MASK)|n1);
      upkey = node->key_array[NTH(node->slot_array,n1+1)];
      upchild = node2;
    }
  }
  if (top >= 0) {
    return;
  }
  assert(upchild != NULL);
  NonLeaf* r2 = new_NonLeaf();
  r2->slot_array = 1; // [1] = 0 and [0] = 1, regardless of SLOTWIDTH
  r2->key_array[0] = upkey;
  r2->pointer_array[0] = upchild;
  r2->pointer_array[MAXKEY] = tree->root;
  // ATOM 2 BEGIN
  // LOG BEGIN: TODO FIXME
  tree->root = r2;
  tree->height++;
  mem_flush(&(tree->root));
  mem_flush(&(tree->height));
  {
    for (int i = 0; i < atom_n; i++) {
      *(atom_addr[i]) = atom_newv[i];
      mem_flush(atom_addr[i]);
    }
    atom_n = 0;
  }
  // LOG END
  assert(atom_n == 0);
  // ATOM 2 END
}

int lookup_internal(Tree* tree, KeyType key, RecordType* rec, Leaf** plf, uint8_t* pslot) {
  Leaf* lf = NULL;
  NonLeaf* p = tree->root;
  for (int i = 0; i < tree->height-1; i++) {
    uint8_t n = SIZE(p);
    uint8_t pos_slot = locate_first(p->key_array, p->slot_array, n, key)-1;
    uint8_t pos_key = (pos_slot >= 1)?NTH(p->slot_array,pos_slot):MAXKEY;
    p = (NonLeaf*) p->pointer_array[pos_key];
  }
  lf = (Leaf*) p;
  uint8_t n = SIZE(lf);
  uint8_t pos_slot = locate_first(lf->key_array, lf->slot_array, n, key);
  while (pos_slot > n || lf->key_array[NTH(lf->slot_array,pos_slot)] < key) {
    if (pos_slot > n) {
      lf = lf -> next;
      if (lf == NULL) {
        return 0;
      }
      n = SIZE(lf);
      pos_slot = 1;
    } else {
      pos_slot++;
    }
  }
  if (lf->key_array[NTH(lf->slot_array,pos_slot)] > key) {
    return 0;
  }
  *rec = lf->record_array[NTH(lf->slot_array,pos_slot)];
  if (plf != NULL) {
    *plf = lf;
    assert(pslot != NULL);
    *pslot = pos_slot;
  }
  return 1;
}
int lookup(Tree* tree, KeyType key, RecordType* rec) {
  return lookup_internal(tree, key, rec, NULL, NULL);
}
int delete(Tree* tree, KeyType key) {
  // empty leaves are left as-is, lookup is clever enough
  RecordType rec;
  Leaf* lf;
  uint8_t pos_slot;
  int ret = lookup_internal(tree, key, &rec, &lf, &pos_slot);
  if (ret == 0) {
    return 0;
  }
  assert(lf->record_array[NTH(lf->slot_array,pos_slot)] == rec);
  assert(lf->key_array[NTH(lf->slot_array,pos_slot)] == key);
  uint64_t tempslot = 0;
  for (int i = SIZE(lf)-1; i >= pos_slot; i--) { SNTH(tempslot, i, NTH(lf->slot_array,i+1)); }
  for (int i = pos_slot-1; i >= 0; i--) { SNTH(tempslot, i, NTH(lf->slot_array,i)); }
  SNTH(tempslot, 0, NTH(tempslot, 0)-1);
  lf->slot_array = tempslot;
  mem_flush(&(lf->slot_array));
  return 1;
}


void print_tree(void* vnode, int height) {
  if (height > 1) {
    NonLeaf* node = (NonLeaf*)vnode;
    uint8_t n = SIZE(node);
    fprintf(stderr, "NonLeaf @ %dR, ", addr2sym(node));
    fprintf(stderr, "(%d, %lx), height = %d\n", n, node->slot_array, height);
    uint8_t rank[MAXKEY] = {0};
    for (uint8_t i = 1; i < MAXSLOT; i++) {
      fprintf(stderr, " %10lu", NTH(node->slot_array,i));
      if (i <= n) {
        rank[NTH(node->slot_array,i)] = i;
      }
    }
    fprintf(stderr, "\n");
    for (uint8_t i = 0; i < MAXKEY; i++) {
      fprintf(stderr, " %7lu/%2d", node->key_array[i], rank[i]);
    }
    fprintf(stderr, "\n");
    for (uint8_t i = 0; i < MAXPOINTER; i++) {
      fprintf(stderr, " %9dR", addr2sym(node->pointer_array[i]));
    }
    fprintf(stderr, "\n");
    print_tree(node->pointer_array[MAXKEY], height-1);
    for (uint8_t i = 1; i <= n; i++) {
      print_tree(node->pointer_array[NTH(node->slot_array,i)], height-1);
    }
  } else {
    Leaf* node = (Leaf*)vnode;
    uint8_t n = SIZE(node);
    fprintf(stderr, "Leaf @ %dR, ", addr2sym(node));
    fprintf(stderr, "(%lu, %lx), sibling = %dR\n", NTH(node->slot_array,0), node->slot_array, addr2sym(node->next));
    uint8_t rank[MAXKEY] = {0};
    for (uint8_t i = 1; i < MAXSLOT; i++) {
      fprintf(stderr, " %10lu", NTH(node->slot_array,i));
      if (i <= n) {
        rank[NTH(node->slot_array,i)] = i;
      }
    }
    fprintf(stderr, "\n");
    for (uint8_t i = 0; i < MAXKEY; i++) {
      fprintf(stderr, " %7lu/%2d", node->key_array[i], rank[i]);
    }
    fprintf(stderr, "\n");
    for (uint8_t i = 0; i < MAXRECORD; i++) {
      fprintf(stderr, " %10lu", node->record_array[i]);
    }
    fprintf(stderr, "\n");
  }
}

void print_lookup(Tree* tree, KeyType key) {
  RecordType rec;
  int ret = lookup(tree, key, &rec);
  if (ret) {
    printf("%lu: %lu\n", key, rec);
  } else {
    printf("%lu: not found\n", key);
  }
}

/* EVAL */

//test1: insert/delete, option1 = sequential, option2 = random
timeTuple test1(Tree* tree, int option) {
  uint64_t diffInsert, diffDelete;
  struct timespec startInsert,endInsert, startDelete, endDelete;
  struct timeTuple ret;

  srand(42);
  //time insert
  clock_gettime(CLOCK_MONOTONIC, &startInsert);
  for (int i = 0; i < NUMINSERT; i++) {
    if(option == 1) insert_ng(tree, i, i*3);
    else if(option == 2){
      insert_ng(tree, rand()%10000, i*3);
    }
  }
  clock_gettime(CLOCK_MONOTONIC, &endInsert);
  diffInsert = BILLION * (endInsert.tv_sec - startInsert.tv_sec) + endInsert.tv_nsec - startInsert.tv_nsec;
  //printf("insert time = %llu ns\n", (long long unsigned int) diffInsert);

  //print_tree(tree->root, tree->height);
  //time delete
  clock_gettime(CLOCK_MONOTONIC, &startDelete);
  for (int i = 0; i < NUMDELETE; i++) {
    delete(tree, i);
  }
  clock_gettime(CLOCK_MONOTONIC, &endDelete);
  diffDelete = BILLION * (endDelete.tv_sec - startDelete.tv_sec) + endDelete.tv_nsec - startDelete.tv_nsec;
  //printf("delete time = %llu ns\n", (long long unsigned int) diffDelete);

  ret.insertTime = diffInsert;
  ret.deleteTime = diffDelete;

  return ret;
  //print_tree(tree->root, tree->height);
}

/*
//test2: single split/delete
timeTuple test2(Tree* tree) {
  uint64_t diffInsert, diffDelete;
  struct timespec startInsert,endInsert, startDelete, endDelete;
  struct timeTuple ret;

  //time single split
  for (int i = 0; i < MAXKEY-1; i++) {
    insert_ng(tree, i, i*3);
  }
  clock_gettime(CLOCK_MONOTONIC, &startInsert);
  insert_ng(tree, MAXKEY, MAXKEY*3);
  clock_gettime(CLOCK_MONOTONIC, &endInsert);
  diffInsert = BILLION * (endInsert.tv_sec - startInsert.tv_sec) + endInsert.tv_nsec - startInsert.tv_nsec;
  printf("insert time = %llu ns\n", (long long unsigned int) diffInsert);
  print_tree(tree->root, tree->height);

  for (int i = 0; i < MAXKEY-1; i++) {
    insert_ng(tree, i, i*3);
  }
  //time delete
  clock_gettime(CLOCK_MONOTONIC, &startDelete);
  delete(tree, MAXKEY);
  clock_gettime(CLOCK_MONOTONIC, &endDelete);
  diffDelete = BILLION * (endDelete.tv_sec - startDelete.tv_sec) + endDelete.tv_nsec - startDelete.tv_nsec;
  printf("delete time = %llu ns\n", (long long unsigned int) diffDelete);

  print_tree(tree->root, tree->height);

  ret.insertTime = diffInsert;
  ret.deleteTime = diffDelete;

  return ret;
  //print_tree(tree->root, tree->height);
}
*/
//test 3 single split
uint64_t test3(Tree* tree) {
  uint64_t diffInsert;
  struct timespec startInsert,endInsert;
 
  //time single split
  for (int i = 0; i < MAXKEY/2; i++) {
    insert_ng(tree, i, i*3);
  }
//  print_tree(tree->root, tree->height);
  clock_gettime(CLOCK_MONOTONIC, &startInsert);
  insert_ng(tree, MAXKEY, MAXKEY*3);
  clock_gettime(CLOCK_MONOTONIC, &endInsert);
  diffInsert = BILLION * (endInsert.tv_sec - startInsert.tv_sec) + endInsert.tv_nsec - startInsert.tv_nsec;
//  printf("insert time = %llu ns\n", (long long unsigned int) diffInsert);
//  print_tree(tree->root, tree->height);

  return diffInsert;
}

uint64_t test4(Tree* tree) {
  uint64_t diffDelete;
  struct timespec startDelete, endDelete;

  for (int i = 0; i < MAXKEY/2+MAXKEY; i++) {
    insert_ng(tree, i, i*3);
  }
//  print_tree(tree->root, tree->height);
  //time delete
  clock_gettime(CLOCK_MONOTONIC, &startDelete);
//  mc_counter = 0;
//  c_counter = 0;
//  m_counter = 0;
  delete(tree, MAXKEY/2);
//  printf("mc = %d, c = %d, m = %d\n", mc_counter, c_counter, m_counter);
  clock_gettime(CLOCK_MONOTONIC, &endDelete);
  diffDelete = BILLION * (endDelete.tv_sec - startDelete.tv_sec) + endDelete.tv_nsec - startDelete.tv_nsec;
//  printf("delete time = %llu ns\n", (long long unsigned int) diffDelete);

//  print_tree(tree->root, tree->height);

  return diffDelete;
}


/* original tests
//manual test
void test1(Tree* tree) {
  insert_ng(tree, 13, 42);
  insert_ng(tree, 1, 42);
  insert_ng(tree, 3, 42);
  insert_ng(tree, 7, 42);
  insert_ng(tree, 19, 42);
  insert_ng(tree, 14, 42);
  insert_ng(tree, 9, 42);

  insert_ng(tree, 2, 42);
  insert_ng(tree, 0, 42);
  insert_ng(tree, 17, 42);
  insert_ng(tree, 22, 42);
  insert_ng(tree, 8, 42);
  insert_ng(tree, 10, 42);

  print_tree(tree->root, tree->height);
}

//rand test
void test2(Tree* tree) {
  srand(42);
  for (int i = 0; i < 10000; i++) {
    int x = rand()%10000;
    insert_ng(tree, x, x % 7);
  }
//  print_tree(tree->root, tree->height);
  print_lookup(tree, 1169);
  print_lookup(tree, 3867);
  print_lookup(tree, 9579);
  print_lookup(tree, 280);
  print_lookup(tree, 6166);
  print_lookup(tree, 9448);
  print_lookup(tree, 4504);
}

//sequential++
void test3(Tree* tree) {
  for (int i = 0; i < 10000; i++) {
    insert_ng(tree, i, i*3);
  }
  print_tree(tree->root, tree->height);
  print_lookup(tree, 21);
  print_lookup(tree, 0);
  print_lookup(tree, 35);
  print_lookup(tree, 36);
  print_lookup(tree, 39);
}

//sequential--
void test4(Tree* tree) {
  for (int i = 3*7+7-1 +1; i >= 0; i--) {
    insert_ng(tree, i, 42);
  }
  print_tree(tree->root, tree->height);
}

//insert duplicate
void test5(Tree* tree) {
  for (int i = 0; i < 35; i++) {
    insert_ng(tree, 4, i);
  }
  print_tree(tree->root, tree->height);
  for (int i = 0; i < 21; i++) {
    delete(tree, 4);
  }
  insert_ng(tree, 4, 0);
  insert_ng(tree, 3, 0);
  print_tree(tree->root, tree->height);
  print_lookup(tree, 4);
  print_lookup(tree, 3);
  print_lookup(tree, 5);
}
*/

int 
main(int argc, char* argv[])
{
//  Tree* t = init_empty_tree();
  struct timeTuple result;
  uint64_t insertTime = 0;
  uint64_t deleteTime = 0;
  uint64_t sum = 0;
  //printf("arg=%d\n",atoi(argv[1]));
  int i = 0;

  switch (*argv[1]) {
  case '1':
  case '2':
    insertTime = 0;
    deleteTime = 0;

    i = 0;
    for(i = 0; i < EVALLOOP; i++){
      Tree* tn = init_empty_tree();
      result = test1(tn,atoi(argv[1]));
      insertTime = insertTime + result.insertTime;
      deleteTime += result.deleteTime;
      //printf("here: sum = %llu, lap = %llu\n", (long long unsigned int) insertTime, (long long unsigned int) result.insertTime);
    }
    insertTime = (insertTime * 1.0)/EVALLOOP;
    deleteTime = (deleteTime * 1.0)/EVALLOOP;
    printf("insert time = %llu ns\n", (long long unsigned int) insertTime);
    printf("delete time = %llu ns\n", (long long unsigned int) deleteTime);
    break;

  case '3':
    insertTime = 0;
    sum = 0;

    i = 0;
    for(i = 0; i < EVALLOOP; i++)
    {
      Tree* tn = init_empty_tree();
      insertTime = test3(tn);
      sum = sum + insertTime;
    }
    insertTime = (sum*1.0)/EVALLOOP;
    printf("split time = %llu ns\n", (long long unsigned int) insertTime);
    break;

  case '4':
    deleteTime = 0;
    sum = 0;

    i = 0;
    for(i = 0; i < EVALLOOP; i++)
    {
      Tree* tn = init_empty_tree();
      deleteTime = test4(tn);
      sum = sum + deleteTime;
    }
    deleteTime = (sum*1.0)/EVALLOOP;
    printf("delete time = %llu ns\n", (long long unsigned int) deleteTime);
    break;

/*
  case '5':
    test5(t);
    break;
*/
  default:
    ;
  }

  return 0;
}
