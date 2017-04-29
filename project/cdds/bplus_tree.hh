#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <limits.h>

#define CacheLineSize 64
#define NumCacheLinesPerNode 4
// TODO
#define BranchFactorInternal (CacheLineSize*NumCacheLinesPerNode - 3*sizeof(void*) - sizeof(bool) )/(sizeof(int) + sizeof(void*))
#define BranchFactorLeaf (CacheLineSize*NumCacheLinesPerNode - sizeof(int) - 4*sizeof(void*) - sizeof(bool) )/(sizeof(int) + sizeof(void*))

// #define utiLimitNonLeaf BranchFactorInternal/5
// #define utiLimitLeaf BranchFactorLeaf/5

// for test
// #define BranchFactorInternal 6
// #define BranchFactorLeaf 6


#define Infinite INT_MAX

extern unsigned mfenceCount;
extern unsigned clflushCount;

// cache line flush
inline void clflush(volatile void *p);

inline void
clflush(volatile void *p)
{
    // for debug
    assert(p != nullptr);

    ++clflushCount;
    asm volatile ("clflush (%0)" :: "r"(p));
}

// mfence
inline void mfence();

inline void 
mfence() 
{
    ++mfenceCount;
    asm volatile("mfence" : : : "memory");
}

class Record 
{
    public:
        int value;
        Record* nextRecord;
        unsigned refCounter;

        bool valid;

        Record(int k, Record* nxt = nullptr)
        {
            value = k;
            nextRecord = nxt;
            refCounter = 0;
            valid = false;
        }

        void setNextRecord(Record* nxt) { nextRecord = nxt; }
        void incRefCounter() { ++refCounter; }
        void decRefCounter() { --refCounter; }
        bool isRefZero()
        {
            if(refCounter == 0)
                return true;
            else 
                return false;
        }
};

void deleteRecordList(Record* record);
void insertRecordList(Record* recordList, Record* record);
void printRecordList(Record* recordList);

// class for leaf node entry
class Entry
{
    public:
        int key;
        // all elements contained in pointer <= key
        void* pointer;
        std::pair<unsigned, unsigned> version;

        bool valid;

        Entry()
        {
            key = 0;
            pointer = nullptr;
            version.first = 1;
            version.second = 1;
            valid = false;
        }
};


// void deallocRecordEntry(Entry* entries, int len);
// void deallocNodeEntry(Entry* entries, int len);

// defined later
class BTree;


// the internal node class
class Node 
{
    public:
        Entry* entries;
        Node * parent;
        bool is_leaf;
        int num_entries;
        int num_live_entries;

        // the btree it belongs to
        BTree* btree;

        // used to chech whether deletion is safe
        unsigned refCounter;

    public:

        Node()
        {
            num_entries = 0;
            num_live_entries = 0;
            is_leaf = false;
            parent = nullptr;

            refCounter = 0;

        }

        virtual void alloc()
        {
            // std::cout << "alloc internal" << std::endl;
            // the last key is padded with Infinite to ease cdds implementation
            entries = new Entry[BranchFactorInternal];
        }

        // dealloc the subtrees, including the records in leaves (dealloc the contents in the pointer array)
        virtual void dealloc()
        {
            for(int i = 0; i < num_entries; ++i)
            {
                Node* n = reinterpret_cast<Node*> (entries[i].pointer);
                assert(n != nullptr);
                n->decRefCounter();
                if(n->isRefZero())
                {
                    n->dealloc();
                    delete n;
                }
            }
        }

        // only delete the array itself. Contents are left
        ~Node()
        {
            delete[] entries;
        }

        void print();

        void clear() { num_entries = 0; }

        void incRefCounter() { refCounter++; }
        void decRefCounter() { refCounter++; }

        bool isRefZero()
        {
            if(refCounter == 0)
                return true;
            else
                return false;
        }

        void setParent(Node* p) 
        {
            // std::cout << "set parent called" << std::endl;
            this->parent = p; 
        }

        void setNumEntries(int n) { this->num_entries = n; }
        void setNumLiveEntries(int n) { this->num_live_entries = n; }

        // return the first node containing the given value, used for scan operations
        // return current node if it is leaf
        virtual Node* lookUp(int value);

        // @inserted_value: value to be inserted into b+ tree
        // @ptr: the ptr to be inserted (pointing to child node)
        // @ newKeyLeft: key coupled with left child
        // @ newKeyRight:key coupled with the right child
        // @new_Node_left: returns the left newNode if split is needed, otherwise returns nulllptr
        // @new_Node_right: returns the right newNode if split is needed, otherwise returns nulllptr
        // Notice that if the to be splitted pointer is the last one in the array (num_entries), needs special treatment
        virtual void insert(int insertedKey, void* ptr, int& newKey, void* & new_Node_left, void* &new_Node_right);

        // used to balance two child nodes of this pointer
        // @pos: idx of the child node needs merging (idx and idx+1)
        // modify keys array in this function
        void balanceChilds(int pos_1, int pos_2); 

        // used to merge two child nodes into one
        // @pos: idx of the child node needs merging (idx and idx+1)
        void mergeChilds(int pos_1, int pos_2);

        // used to merge child nodes of this pointer after remove operation
        // @pos: idx of the child node needs merging (idx and idx+1)
        void merge(int pos);

        // delete an element from b+ tree
        // @rmKey: the key to be deleted
        // returns if the child node needs merging
        virtual bool remove(int rmKey);

        virtual void setNextNode(Node* ptr) { return; }
        virtual void setPreviousNode(Node* ptr) { return; }
        virtual void scan(int key) { return; }
        virtual void scan(int lower, int upper) { return; }

        void setBtree(BTree* b) { btree = b; }

    protected:
        int reusableEntry();

        // find two reusable entries at the same time
        // writes to pos_1 & pos_2
        // returns true on success
        bool reuseEntriesTwo(int& pos_1, int& pos_2);
        // used when reusing dead entry in leaf node, 
        // @pos is the last
        // @len: length of the array (might == num_entries + 1 if we are inserting to a new location)
        void sortLeftward(int pos, Entry* arr);
        void sortRightward(int pos, Entry* arr, int len);

        void sortLeftwardNoFlush(int pos, Entry* arr);

        void setEndVersion();

        int findLiveEntryLeft(int pos)
        {
            for(int i = pos - 1; i >= 0; --i)
            {
                if(entries[i].version.second == 0)
                {
                    assert(entries[i].valid);
                    return i;
                }
            }
            return -1;
        }

        int findLiveEntryRight(int pos)
        {
            for(int i = pos + 1; i < num_entries; ++i)
            {
                if(entries[i].version.second == 0)
                {
                    assert(entries[i].valid);
                    return i;
                }
            }
            return -1;
        }
};

class LeafNode: public Node
{
    public:
        LeafNode* prevNode;
        LeafNode* nextNode;
        LeafNode() 
        {   
            // num_entries = 0;
            // num_live_entries = 0;
            is_leaf = true;
            prevNode = nullptr;
            nextNode = nullptr;

            // parent = nullptr;
        }

        void alloc()
        {
            // std::cout << "alloc leaf" << std::endl;
            entries = new Entry[BranchFactorLeaf];
        }

        void dealloc()
        {
            for(int i = 0; i < num_entries; ++i)
            {
                // std::cout << "~LeafNode i = " << i << std::endl;
                assert(entries[i].pointer != nullptr);
                Record* r = reinterpret_cast<Record*> (entries[i].pointer);
                r->decRefCounter();
                if(r->isRefZero())
                    deleteRecordList(r);
            }
        }

        // destructor same as Node
        // ~LeafNode()
        // {
        //     num_entries = 0;
        // }

        Node* lookUp(int value);

        void insert(int insertedKey, void* ptr, int& newKey, void* & new_Node_left, void* &new_Node_right);

        // delete an element from b+ tree
        // @rmKey: the key to be deleted
        // returns if the child node needs merging
        bool remove(int rmKey);

        // output all data within given bounds in the tree
        void scan(int lower, int upper);

        // give back records ontaining the key
        void scan(int key);

        void setNextNode(Node* ptr) { this->nextNode = dynamic_cast<LeafNode*> (ptr); }
        void setPrevNode(Node* ptr) { this->prevNode = dynamic_cast<LeafNode*> (ptr); }
};


class BTree
{
    public:
        Node* root;
        unsigned rootStartVersion;

        BTree()
        {
            root = new LeafNode;
            root->alloc();
            root->setBtree(this);
            rootStartVersion = 1;
        }

        ~BTree()
        {
            root->~Node();
        }

    private:
        unsigned version;

    public:
        void insert(int key);
        void remove(int key);
        void lookUp(int key);
        void lookUp(int lower, int upper);
        unsigned getVersion() { return version; }
        void incVersion() { ++version; }

        void flushVersion() 
        { 
            mfence();
            clflush(&version); 
            mfence();
        }
};




