#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <assert.h>

using namespace std;

#define CacheLineSize 64
#define NumCacheLinesPerNode 4
// TODO
#define BranchFactorInternal (CacheLineSize*NumCacheLinesPerNode - sizeof(int) + sizeof(int) - sizeof(bool) )/(sizeof(int) + sizeof(void*))
#define BranchFactorLeaf (CacheLineSize*NumCacheLinesPerNode - sizeof(int) - sizeof(void*) - sizeof(bool) )/(sizeof(int) + sizeof(void*))

// for test
// #define BranchFactorInternal 5
// #define BranchFactorLeaf 5


class Record 
{
    public:
        int value;
        Record* nextRecord;

        Record(int k, Record* nxt = nullptr)
        {
            value = k;
            nextRecord = nxt;
        }

        void setNextRecord(Record* nxt) { nextRecord = nxt; }
};

void deleteRecordList(Record* record);
void insertRecordList(Record* recordList, Record* record);
void printRecordList(Record* recordList);


// the internal node class
class Node 
{
    public:
        void ** pointers;
        // ele < key | key | ele >= key
        int* keys;
        // Node * parent;
        bool is_leaf;
        int num_keys;
        // int space;

        // // for cdds
        // int* key_version;
        // int* ptr_version;

        Node()
        {
            num_keys = 0;
            is_leaf = false;
            // parent = nullptr;
        }

        virtual void alloc()
        {
            // std::cout << "alloc internal" << std::endl;
            keys = new int[BranchFactorInternal - 1];
            pointers = new void*[BranchFactorInternal];
            for(int i = 0; i < BranchFactorInternal; ++i)
                pointers[i] = nullptr;
        }

        ~Node()
        {
            // std::cout << "is_leaf = " << is_leaf << std::endl;
            // std::cout << "num_keys = " << num_keys << std::endl;
            if(!is_leaf)
            {
                if(num_keys > 0)
                {
                    for(int i = 0; i <= num_keys; ++i)
                    {
                        
                        // std::cout << "~Node i = " << i << std::endl;
                        Node* n = reinterpret_cast<Node*> (pointers[i]);
                        assert(n != nullptr);
                        n->~Node();
                    }
                }
            }

            // std::cout << "delete pointers & keys in Node" << std::endl;
            delete[] pointers;
            delete[] keys;
            // std::cout << "~Node i done " << std::endl;
        }

        void clear() { num_keys = 0; }

        // void setParent(Node* p) 
        // {
        //     // std::cout << "set parent called" << std::endl;
        //     this->parent = p; 
        // }

        void setNumKeys(int n) { this->num_keys = n; }

        // return the first node containing the given value, used for scan operations
        // return current node if it is leaf
        virtual Node* lookUp(int value);

        // @inserted_value: value to be inserted into b+ tree
        // @ptr: the ptr to be inserted (pointing to child node)
        // @ newKey: key to divide the two nodes after split
        // @new_Node: returns the newNode if split is needed, otherwise returns nulllptr
        virtual void insert(int insertedKey, void* ptr, int& newKey, void *& new_Node);

        // used to balance two child nodes of this pointer
        // @pos: idx of the child node needs merging (idx and idx+1)
        // modify keys array in this function
        void balanceChilds(int pos); 

        // used to merge two child nodes into one
        // @pos: idx of the child node needs merging (idx and idx+1)
        void mergeChilds(int pos);

        // used to merge child nodes of this pointer after remove operation
        // @pos: idx of the child node needs merging (idx and idx+1)
        void merge(int pos);

        // delete an element from b+ tree
        // @rmKey: the key to be deleted
        // returns if the child node needs merging
        virtual bool remove(int rmKey);

        virtual void setNextNode(Node* ptr) { return; }
        virtual void scan(int) { return; }
        virtual void scan(int, int) { return; }
};

class LeafNode: public Node
{
    public:
        LeafNode* nextNode;
        LeafNode() 
        {   
            num_keys = 0;
            is_leaf = true;
            // parent = nullptr;
            // prevNode = nullptr;
            nextNode = nullptr;
        }

        void alloc()
        {
            // std::cout << "alloc leaf" << std::endl;
            keys = new int[BranchFactorLeaf];
            pointers = new void*[BranchFactorLeaf];
            // std::cout << "BranchFactorLeaf = " << BranchFactorLeaf << std::endl;
            for(int i = 0; i < BranchFactorLeaf; ++i)
                pointers[i] = nullptr;
        }

        ~LeafNode()
        {
            if(num_keys > 0)
            {
                for(int i = 0; i < num_keys; ++i)
                {
                    // std::cout << "~LeafNode i = " << i << std::endl;
                    assert(pointers[i] != nullptr);
                    Record* r = reinterpret_cast<Record*> (pointers[i]);
                    deleteRecordList(r);
                }
            }
            num_keys = 0;
        }

        Node* lookUp(int value);

        void insert(int insertedKey, void* ptr, int& newKey, void*& new_Node);

        // delete an element from b+ tree
        // @rmKey: the key to be deleted
        // returns if the child node needs merging
        bool remove(int rmKey);

        // output all data within given bounds in the tree
        void scan(int lower, int upper);

        // give back records ontaining the key
        void scan(int key);

        void setNextNode(Node* ptr) { this->nextNode = dynamic_cast<LeafNode*> (ptr); }
        // void setPrevNode(Node* ptr) { this->prevNode = dynamic_cast<LeafNode*> (ptr); }
};


class BTree
{
    public:
        Node* root;

        BTree()
        {
            root = new LeafNode();
            root->alloc();
        }

        ~BTree()
        {
            root->~Node();
        }

        void insert(int key);
        void remove(int key);
        void lookUp(int key);
        void lookUp(int lower, int upper);
};




