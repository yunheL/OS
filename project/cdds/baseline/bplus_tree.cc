#include "bplus_tree.hh"

void deleteRecordList(Record* record)
{
    if(record->nextRecord == nullptr)
        delete record;
    else
    {
        deleteRecordList(record->nextRecord);
        delete record;
    }
}

// insert a record into the record list
void insertRecordList(Record* recordList, Record* record)
{
    Record* r = recordList;
    while(r->nextRecord != nullptr)
        r = r->nextRecord;
    r->nextRecord = record;
    record->nextRecord = nullptr;
}


void printRecordList(Record* recordList)
{
    while(recordList != nullptr)
    {
        std::cout << recordList->value << " ";
        recordList = recordList->nextRecord;
    }
    std::cout << std::endl;
}

Node*
Node::lookUp(int value)
{
    // std::cout << "in lookUp num_keys = " << num_keys << std::endl;
    int i;
    for(i = 0; i < num_keys; ++i)
    {
        // std::cout << "keys[" << i << "] = " << keys[i] << std::endl;
        if(keys[i] > value)
        {
            break;
        }
    }
    // std::cout << "lookUp pos = " << i << std::endl;
    assert(pointers[i] != nullptr);
    Node* n = reinterpret_cast<Node*> (pointers[i]);
    assert(n != nullptr);
    n->lookUp(value);
    // std::cout << "lookUp done" << std::endl;
    // return reinterpret_cast<Node*> (pointers[i]);
}

Node*
LeafNode::lookUp(int value)
{
    return this;
}


void 
Node::insert(int insertedKey, void* ptr, int& newKey, void *& new_Node)
{
    // std::cout << "node insert called" << std::endl;
    // std::cout << "in node insert is_leaf = " << is_leaf << std::endl;
    // for internal nodes
    // find the child node that the key could map to
    // pos of the selected ptr
    int pos;
    for(pos = 0; pos < num_keys; ++pos)
    {
        if(keys[pos] > insertedKey)
            break;
    }

    assert(pointers[pos] != nullptr);
    void* newNodePtr;
    int splitKey;
    Node* child = reinterpret_cast<Node*> (pointers[pos]);
    child->insert(insertedKey, ptr, splitKey, newNodePtr);
    if(newNodePtr == nullptr)
    {
        // no split is done at child's level, safe to return
        new_Node = nullptr;
        return;
    }
    else
    {
        // std::cout << "return to parent node for split, pos = " << pos << std::endl;
        // split at child's level, insert the newNodePtr and splitKey into this node
        if(num_keys == BranchFactorInternal - 1)
        {
            // current node needs splitting
            // construct an temporary array
            void** tempPtrArr = new void*[BranchFactorInternal + 1];
            int* tempKeyArr = new int[BranchFactorInternal];
            for(int i = 0; i < pos; ++i)
            {
                // std::cout << "assigning tempKeyArr idx = " << i << std::endl;
                // std::cout << "assigning tempPtrArr idx = " << i << std::endl;

                tempKeyArr[i] = keys[i];
                tempPtrArr[i] = pointers[i];
            }
        
            for(int i = pos + 1; i < BranchFactorInternal; ++i)
            {
                // std::cout << "in second loop assigning tempKeyArr idx = " << i << std::endl;
                // std::cout << "in second loop assigning tempPtrArr idx = " << i + 1 << std::endl;

                tempKeyArr[i] = keys[i - 1];
                tempPtrArr[i + 1] = pointers[i];
            }
            // tempPtrArr[BranchFactorInternal] = pointers[BranchFactorInternal - 1];

            // std::cout << "after loop assigning tempKeyArr idx = " << pos << std::endl;
            // std::cout << "after loop assigning tempPtrArr idx = " << pos + 1 << std::endl;
            // std::cout << "tempPtrArr idx = " << pos << " no change" << std::endl;

            // tempPtrArr[pos] does not change
            tempPtrArr[pos + 1] = newNodePtr;
            tempPtrArr[pos] = pointers[pos];
            tempKeyArr[pos] = splitKey;


            // for debug
            // for(int i = 0; i <= BranchFactorInternal; ++i)
            // {
            //     Node* ln = reinterpret_cast<Node*> (tempPtrArr[i]);
            //     assert(ln != nullptr);
            //     std::cout << "is temp entry is_leaf =  " << ln->is_leaf  << ", idx = " << i << std::endl;
            //     std::cout << "BranchFactorInternal = " << BranchFactorInternal << std::endl;
            // }
            //

            Node* newNode = new Node;
            newNode->alloc();
            // the parition key index, used as newKey returned to parent
            int partitionIdx = BranchFactorInternal/2;
            int numKeysLeft = BranchFactorInternal/2;
            int numKeysRight = BranchFactorInternal - BranchFactorInternal/2 - 1;
            // set newKey (split key) to update parent
            newKey = tempKeyArr[partitionIdx];
            // std::cout << "after partition key in Node::insert = " << newKey << std::endl;
            for(int i = pos; i < numKeysLeft; ++i)
                keys[i] = tempKeyArr[i];
            for(int i = pos; i <= numKeysLeft; ++i)
                pointers[i] = tempPtrArr[i];

            // std::cout << "after assigning left child in Node::insert" << std::endl;

            for(int i = 0; i < numKeysRight; ++i)
            {
                // std::cout << "assigning rightChild keys i = " << i << std::endl;
                newNode->keys[i] = tempKeyArr[i + numKeysLeft + 1];
            }
                
            for(int i = 0; i <= numKeysRight; ++i)
            {
                // for debug
                // std::cout << "assigning rightChild ptr i = " << i << std::endl;
                // std::cout << "tempPtrArr idx = " << i + numKeysLeft + 1 << std::endl;
                Node* n = reinterpret_cast<Node*> (tempPtrArr[i + numKeysLeft + 1]);
                assert(n != nullptr);
                // std::cout << "is temp entry is_leaf =  " << n->is_leaf << std::endl;
                ///
                newNode->pointers[i] = tempPtrArr[i + numKeysLeft + 1];

                // set parent for childs belonging to the new node
                // Node* rightChild = reinterpret_cast<Node*>(newNode->pointers[i]);
                // rightChild->setParent(newNode);
            }

            // std::cout << "after assigning right child in Node::insert" << std::endl;

            setNumKeys(numKeysLeft);
            newNode->setNumKeys(numKeysRight);

            // set parent for child level newly allocated node if it belongs to the left(originial) node after split
            // Node* newChildNode = reinterpret_cast<Node*> (newNodePtr);
            // if(pos <= numKeysLeft)
                // newChildNode->setParent(this);
            // 
            // set new_Node and newKey return value to update current node's parent
            new_Node = reinterpret_cast<void*> (newNode);

            delete[] tempKeyArr;
            delete[] tempPtrArr;
        }
        else
        {
            // no split needed at this node
            // std::cout << "in node insert no parent split branch" << std::endl;
            for(int i = num_keys; i > pos; --i)
                keys[i] = keys[i - 1];

            for(int i = num_keys + 1; i > pos + 1; --i)
                pointers[i] = pointers[i - 1];

            keys[pos] = splitKey;
            pointers[pos + 1] = newNodePtr;

            setNumKeys(num_keys + 1);
            new_Node = nullptr;
            // set parent for the new node
            Node* newNode = reinterpret_cast<Node*> (newNodePtr);
            // newNode->setParent(this);
        }
    }
}

void 
LeafNode::insert(int insertedKey, void* ptr, int& newKey, void *& new_Node)
{
    // find the pos that the leaf should be in
    // move all ele >= pos one location rightward
    // std::cout << "in leaf node insert " << insertedKey << std::endl;
    int pos;
    for(pos = 0; pos < num_keys; ++pos)
    {
        if(keys[pos] == insertedKey)
        {
            // deal with duplicate key here
            Record* recordList = reinterpret_cast<Record*> (pointers[pos]);
            Record* r = reinterpret_cast<Record*> (ptr);
            insertRecordList(recordList, r);
            new_Node = nullptr;

            // for debug
            // std::cout << "leaf node inser done" << std::endl;
            return;
        }
        else if(keys[pos] > insertedKey)
            break;
    }
    // std::cout << "pos = " << pos << std::endl;
    LeafNode* newNode = nullptr;
    if(num_keys == BranchFactorLeaf)
    {
        // need to split and allocate a new leaf node
        newNode = new LeafNode();
        newNode->alloc();
        LeafNode* next_node = (dynamic_cast<LeafNode*> (this))->nextNode;
        newNode->setNextNode(next_node);
        (dynamic_cast<LeafNode*> (this))->setNextNode(newNode);

        // all idx >= partitionIdx are moved into the new node
        int partitionIdx = (BranchFactorLeaf + 1)/2;
        int numKeysLeft = (BranchFactorLeaf + 1)/2;
        int numKeysRight = BranchFactorLeaf + 1 - (BranchFactorLeaf + 1)/2;
        if(pos >= partitionIdx)
        {
            int posInNewNode = pos - partitionIdx;
            for(int i = 0; i < posInNewNode; ++i)
            {
                newNode->keys[i] = keys[i + numKeysLeft];
                newNode->pointers[i] = pointers[i + numKeysLeft];
            }
            newNode->keys[posInNewNode] = insertedKey;
            newNode->pointers[posInNewNode] = ptr;
            for(int i = posInNewNode + 1; i < numKeysRight; ++i)
            {
                newNode->keys[i] = keys[i + numKeysLeft - 1];
                newNode->pointers[i] = pointers[i + numKeysLeft - 1];
            }
            // TODO: do we need to set the parent here
        }
        else
        {
            for(int i = 0; i < numKeysRight; ++i)
            {
                newNode->keys[i] = keys[i + numKeysLeft - 1];
                newNode->pointers[i] = pointers[i + numKeysLeft - 1];
            }

            for(int i = numKeysLeft - 1; i > pos; --i)
            {
                keys[i] = keys[i - 1];
                pointers[i] = pointers[i - 1];
            }
            keys[pos] = insertedKey;
            pointers[pos] = ptr;
        }
        setNumKeys(numKeysLeft);
        newNode->setNumKeys(numKeysRight);
        newKey = newNode->keys[0];
        new_Node = reinterpret_cast<void*> (newNode);
        // std::cout << "leaf insert done w/ split" << std::endl;
    }
    else
    {
        // no split needed
        // std::cout << "here 1" << std::endl;
        for(int i = num_keys; i > pos && i > 0; --i)
        {
            // std::cout << "executed" << std::endl;
            keys[i] = keys[i - 1];
            pointers[i] = pointers[i - 1];
        }
        // std::cout << "here 2" << std::endl;
        this->keys[pos] = insertedKey;
        this->pointers[pos] = ptr;
        // std::cout << "here 3" << std::endl;
        new_Node = nullptr;
        setNumKeys(num_keys + 1);
        // std::cout << "leaf insert done w/o split" << std::endl;
    }

    // for debug
    // std::cout << "leaf node insert done" << std::endl;
}

void
Node::balanceChilds(int pos)
{
    // for debug
    // std::cout << "in balanceChilds" << std::endl;

    Node* leftChild = reinterpret_cast<Node*> (pointers[pos]);
    Node* rightChild = reinterpret_cast<Node*> (pointers[pos + 1]);

    // for debug
    assert(leftChild != nullptr);
    assert(rightChild != nullptr);

    if(leftChild->is_leaf)
    {
        // std::cout << "in balanceChilds for leaf childs" << std::endl;

        assert(rightChild->is_leaf);
        int numKeysLeft = leftChild->num_keys;
        int numKeysRight = rightChild->num_keys;

        int size = numKeysLeft + numKeysRight;
        // size after balance
        int numKeysLeftNew = size/2;
        int numKeysRightNew = size - size/2;

        void** tempPtrArr = new void*[size];
        int* tempKeyArr = new int[size];
        for(int i = 0; i < numKeysLeft; ++i)
        {
            tempPtrArr[i] = leftChild->pointers[i];
            tempKeyArr[i] = leftChild->keys[i];
        }
        for(int i = numKeysLeft; i < size; ++i)
        {
            tempPtrArr[i] = rightChild->pointers[i - numKeysLeft];
            tempKeyArr[i] = rightChild->keys[i - numKeysLeft];
        }

        /**** for debug  ***/
        // std::cout << "in balanceChilds for leaf, temp array" << std::endl;
        // std::cout << "keys" << std::endl;
        // for(int i = 0; i < size; ++i)
        // {
        //     std::cout << tempKeyArr[i] << " ";
        // }
        // std::cout << std::endl;

        /******/

        for(int i = numKeysLeft; i < numKeysLeftNew; ++i)
        {
            leftChild->pointers[i] = tempPtrArr[i];
            leftChild->keys[i] = tempKeyArr[i];
        }

        for(int i = 0; i < numKeysRightNew; ++i)
        {
            rightChild->pointers[i] = tempPtrArr[i + numKeysLeftNew];
            rightChild->keys[i] = tempKeyArr[i + numKeysLeftNew];
        }

        // set num_keys for balanced childs
        leftChild->setNumKeys(numKeysLeftNew);
        rightChild->setNumKeys(numKeysRightNew);

        // modify the current node's keys
        keys[pos] = rightChild->keys[0];

        delete[] tempKeyArr;
        delete[] tempPtrArr;

        // std::cout << "balanceChilds done for leaf childs" << std::endl;
    }
    else
    {
        // std::cout << "in balanceChilds for non-leaf childs" << std::endl;

        // child is non-leaf node
        assert(!rightChild->is_leaf);
        int numKeysLeft = leftChild->num_keys;
        int numKeysRight = rightChild->num_keys;

        // the additional 1 comes from the parent key that splits the two nodes
        int keySize = numKeysLeft + numKeysRight + 1;
        int ptrSize = keySize + 1;
        // key in the parent that splits the nodes
        int parentKey = keys[pos];

        // minus 1 in numKeysRightNew for new key in parent
        int numKeysLeftNew = keySize/2;
        int numKeysRightNew = keySize - keySize/2 - 1;

        int* tempKeyArr = new int[keySize];
        void** tempPtrArr = new void*[ptrSize];

        // copy pointers and keys from two nodes to temp array
        for(int i = 0; i < numKeysLeft; ++i)
            tempKeyArr[i] = leftChild->keys[i];
            
        for(int i = 0; i <= numKeysLeft; ++i)
            tempPtrArr[i] = leftChild->pointers[i];

        // insert parent key into temp array for re-balance
        tempKeyArr[numKeysLeft] = parentKey;

        /** for debug **/
        assert(parentKey > tempKeyArr[numKeysLeft - 1]);
        assert(parentKey <= rightChild->keys[0]);
        /**************/

        // copy righ child into the temp array
        for(int i = numKeysLeft + 1; i < keySize; ++i)
            tempKeyArr[i] = rightChild->keys[i - numKeysLeft - 1];

        for(int i = numKeysLeft + 1; i <= keySize; ++i)
            tempPtrArr[i] = rightChild->pointers[i - numKeysLeft - 1];

        // new key to divide the balanced nodes
        int newKey = tempKeyArr[numKeysLeftNew];

        // left child
        for(int i = numKeysLeft; i < numKeysLeftNew; ++i)
            leftChild->keys[i] = tempKeyArr[i];
        
        for(int i = numKeysLeft + 1; i <= numKeysLeftNew; ++i)
        {
            leftChild->pointers[i] = tempPtrArr[i];
            // Node* n = reinterpret_cast<Node*> (leftChild->pointers[i]);
            // n->setParent(leftChild);
        }

        // right child
        for(int i = 0; i < numKeysRightNew; ++i)
            rightChild->keys[i] = tempKeyArr[i + numKeysLeftNew + 1];

        for(int i = 0; i <= numKeysRightNew; ++i)
        {
            rightChild->pointers[i] = tempPtrArr[i + numKeysLeftNew + 1];
            // set parent
            // Node* n = reinterpret_cast<Node*> (rightChild->pointers[i]);
            // n->setParent(rightChild);
        }

        // set num_keys for balanced kids
        leftChild->setNumKeys(numKeysLeftNew);
        rightChild->setNumKeys(numKeysRightNew);

        // update keys of current node
        keys[pos] = newKey;

        // delete temp array
        delete[] tempKeyArr;
        delete[] tempPtrArr;

        // std::cout << "balanceChilds done for non-leaf childs" << std::endl;
    }
    // no need to set num_keys for current node
}

void
Node::mergeChilds(int pos)
{
    // std::cout << "in mergeChilds" << std::endl;

    // need to update num_keys for current node
    Node* leftChild = reinterpret_cast<Node*> (pointers[pos]);
    Node* rightChild = reinterpret_cast<Node*> (pointers[pos + 1]);
    if(leftChild->is_leaf)
    {
        // for debug
        // std::cout << "mergeChilds for leaf childs" << std::endl;

        // child is leaf node
        assert(rightChild->is_leaf);
        int numKeysLeft = leftChild->num_keys;
        int numKeysRight = rightChild->num_keys;
        int numKeysNew = numKeysLeft + numKeysRight;
        for(int i = numKeysLeft; i < numKeysNew; ++i)
        {
            leftChild->keys[i] = rightChild->keys[i - numKeysLeft];
            leftChild->pointers[i] = rightChild->pointers[i - numKeysLeft];
        }

        // record the nextNode of rightChild
        LeafNode* rightNextNode = (reinterpret_cast<LeafNode*> (rightChild))->nextNode;
        // set nextNode for child leaf nodes
        (reinterpret_cast<LeafNode*> (leftChild))->setNextNode(rightNextNode);

        // set num_keys for childs
        leftChild->setNumKeys(numKeysNew);
        rightChild->clear();
        delete rightChild;

        // update parent's pointers and keys array
        for(int i = pos; i < num_keys - 1; ++i)
            keys[i] = keys[i + 1];
        for(int i = pos + 1; i < num_keys; ++i)
            pointers[i] = pointers[i + 1];

        // update num_keys for parent
        setNumKeys(num_keys - 1);

        // std::cout << "mergeChilds for leaf childs done" << std::endl;
    }
    else
    {
        // for debug
        // std::cout << "mergeChilds for non-leaf childs" << std::endl;

        // child is non-leaf node
        assert(!rightChild->is_leaf);

        // parent's key to split two childs
        int parentKey = keys[pos];

        int numKeysLeft = leftChild->num_keys;
        int numKeysRight = rightChild->num_keys;
        // plus 1 for key from parent
        int numKeysNew = numKeysLeft + numKeysRight + 1;

        /* for debug */
        assert(numKeysNew <= BranchFactorInternal - 1);
        /*********/

        leftChild->keys[numKeysLeft] = parentKey;
        for(int i = numKeysLeft + 1; i < numKeysNew; ++i)
            leftChild->keys[i] = rightChild->keys[i - numKeysLeft - 1];

        for(int i = numKeysLeft + 1; i <= numKeysNew; ++i)
        {
            leftChild->pointers[i] = rightChild->pointers[i - numKeysLeft - 1];
            // update parent for child's childs
            // Node* n = reinterpret_cast<Node*> (leftChild->pointers[i]);
            // n->setParent(leftChild);
        }
        // update num_keys for childs
        leftChild->setNumKeys(numKeysNew);
        rightChild->clear();
        delete rightChild;

        // update parent for child's childs
        for(int i = pos; i < num_keys - 1; ++i)
            keys[i] = keys[i + 1];

        for(int i = pos + 1; i < num_keys; ++i)
            pointers[i] = pointers[i + 1];

        // update num_keys for parent
        setNumKeys(num_keys - 1);

        // for debug
        // std::cout << "mergeChilds for non-leaf childs done" << std::endl;
    }

}

void 
Node::merge(int pos)
{
    // std::cout << "in merge" << std::endl;

    if(pos == num_keys)
        pos--;

    Node* leftChild = reinterpret_cast<Node*> (pointers[pos]);
    Node* rightChild = reinterpret_cast<Node*> (pointers[pos + 1]);
    int numKeysLeft = leftChild->num_keys;
    int numKeysRight = rightChild->num_keys;

    bool isChildLeaf = false;
    if(leftChild->is_leaf)
        isChildLeaf = true;

    if(isChildLeaf)
    {
        if(numKeysLeft + numKeysRight < BranchFactorLeaf)
            mergeChilds(pos);
        else
            balanceChilds(pos);
    }
    else
    {
        // plus 1 for key from parent (the current node)
        if(numKeysLeft + numKeysRight + 1 < BranchFactorInternal - 1)
            mergeChilds(pos);
        else
            balanceChilds(pos);
    }

    // std::cout << "in merge done" << std::endl;
}

bool 
Node::remove(int rmKey)
{
    // std::cout << "in Node remove, key = " << rmKey << std::endl;
    // find child node idx that could map to rmKey 
    int pos;
    for(pos = 0; pos < num_keys; ++pos)
    {
        if(keys[pos] > rmKey)
            break;
    }

    assert(pointers[pos] != nullptr);
    Node* child = reinterpret_cast<Node*> (pointers[pos]);
    // for debug
    assert(child != nullptr);
    //
    bool needsMerging = child->remove(rmKey);
    if(needsMerging)
        merge(pos);

    // std::cout << "Node remove done, key = " << rmKey << std::endl;

    if(num_keys < BranchFactorInternal/2)
        return true;
    else
        return false;
}

bool 
LeafNode::remove(int rmKey)
{
    // std::cout << "in LeafNode remove, key = " << rmKey << std::endl;

    int idx;
    for(idx = 0; idx < num_keys; ++idx)
    {
        if(keys[idx] == rmKey)
            break;
    }
    if(idx == num_keys)
        return false;

    Record* record = reinterpret_cast<Record*> (pointers[idx]);

    // std::cout << "before deleteRecordList" << std::endl;
    assert(record != nullptr);
    // printRecordList(record);
    deleteRecordList(record);
    // std::cout << "after deleteRecordList" << std::endl;
    // #num keys after removing
    int n = num_keys - 1;
    for(int i = idx; i < n; ++i)
    {
        keys[i] = keys[i + 1];
        pointers[i] = pointers[i + 1];
    }
    setNumKeys(n);

    // std::cout << "LeafNode remove done, key = " << rmKey << std::endl;

    if(n < BranchFactorLeaf/2)
        return true;
    else
        return false;
}

void
LeafNode::scan(int lower, int upper)
{
    LeafNode* ptr = this;
    while(ptr != nullptr)
    {
        int num = ptr->num_keys;
        for(int i = 0; i < num; ++i)
        {
            if(ptr->keys[i] >= lower && ptr->keys[i] < upper)
            {
                Record* r = reinterpret_cast<Record*> (ptr->pointers[i]);
                // printRecordList(r);
            }
            else if(ptr->keys[i] >= upper)
                return;
        }
        ptr = ptr->nextNode;
    }
}

void
LeafNode::scan(int key)
{
    for(int i = 0; i < num_keys; ++i)
    {
        if(keys[i] == key)
        {
            Record* r = reinterpret_cast<Record*> (pointers[i]);
            // printRecordList(r);
            return;
        }
    }
}

void
BTree::insert(int key)
{
    Record* r = new Record(key);
    void* ptr = reinterpret_cast<void*> (r);
    int newKey;
    void* new_Node;
    root->insert(key, ptr, newKey, new_Node);
    // std::cout << "after root insert root->isLeaf" << std::endl;

    if(new_Node == nullptr)
        return;
    else
    {
        // std::cout << "in make a root" << std::endl;
        // make a new root
        Node* newRoot = new Node;
        newRoot->alloc();
        newRoot->setNumKeys(1);
        newRoot->keys[0] = newKey;
        void* oldRoot = reinterpret_cast<void*> (root);
        newRoot->pointers[0] = root;
        newRoot->pointers[1] = new_Node;
        root = newRoot;
    }
}

void
BTree::remove(int key)
{
    root->remove(key);
}

void 
BTree::lookUp(int key)
{
    LeafNode* n = dynamic_cast<LeafNode*>(root->lookUp(key));
    n->scan(key);
}

void 
BTree::lookUp(int lower, int upper)
{
    LeafNode* n = dynamic_cast<LeafNode*>(root->lookUp(lower));
    n->scan(lower, upper);
}