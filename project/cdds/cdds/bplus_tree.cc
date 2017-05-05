#include "bplus_tree.hh"

unsigned mfenceCount = 0;
unsigned clflushCount = 0;

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

void
Node::print()
{
    std::cout << "in node " << this << std::endl;
    std::cout << "is_leaf = " << is_leaf << std::endl;
    for(int i = 0; i < num_entries; ++i)
    {
        std::cout << "entries[" << i << "].key = " << entries[i].key << ", version.first = "
                  << entries[i].version.first << ", version.second = " << entries[i].version.second << std::endl;
    }
    std::cout << std::endl;
}

Node*
Node::lookUp(int value)
{
    // std::cout << "in Node::lookUp for " << value << std::endl;
    // std::cout << "in lookUp num_entries = " << num_entries << std::endl;
    int i;
    for(i = 0; i < num_entries; ++i)
    {
        // std::cout << "keys[" << i << "] = " << keys[i] << std::endl;
        if(entries[i].key >= value && entries[i].version.second == 0)
        {
            // std::cout << "find key = " << entries[i].key << std::endl;
            break;
        }
    }
    // std::cout << "lookUp pos = " << i << std::endl;
    assert(entries[i].pointer != nullptr);
    Node* n = reinterpret_cast<Node*> (entries[i].pointer);
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
Node::setEndVersion()
{
    int count = 0;
    for(int i = 0; i < num_entries; ++i)
    {
        entries[i].version.second = btree->getVersion();
        count += sizeof(Entry);
        if(count == CacheLineSize)
        {
            clflush(&(entries[i].version));
            count -= CacheLineSize;
        }
    }
    if(count != 0)
        clflush(&(entries[num_entries - 1].version));

    setNumLiveEntries(0);
    clflush(&num_live_entries);
}


int
Node::reusableEntry()
{
    for(int i = 0; i < num_entries; ++i)
    {
        if(entries[i].version.second != 0)
            return i;
    }
    return -1;
}

bool
Node::reuseEntriesTwo(int& pos_1, int& pos_2)
{
    int i;
    bool match_1 = false;
    bool match_2 = false;
    for(i = 0; i < num_entries; ++i)
    {
        if(entries[i].version.second != 0)
        {
            pos_1 = i;
            match_1 = true;
            break;
        }
    }

    ++i;
    for(; i < num_entries; ++i)
    {
        if(entries[i].version.second != 0)
        {
            pos_2 = i;
            match_2 = true;
            break;
        }
    }

    return (match_1 && match_2);
}

void 
Node::sortLeftward(int pos, Entry* arr)
{
    int count = 0;
    mfence();
    while(pos > 0 && arr[pos - 1].key > arr[pos].key)
    {
        std::swap(arr[pos], arr[pos - 1]);
        // TODO:
        // do we need to do a flush here
        count += sizeof(Entry);
        if(count >= CacheLineSize)
        {
            //for debug
            // std::cout << "clflush in sortLeftward 1" << std::endl;

            clflush(&(arr[pos]));
            count -= CacheLineSize;
        }
        pos--;
    }
    if(count != 0)
        clflush(&(arr[pos]));
    mfence();
}

void 
Node::sortLeftwardNoFlush(int pos, Entry* arr)
{
    while(pos > 0 && arr[pos - 1].key > arr[pos].key)
    {
        std::swap(arr[pos], arr[pos - 1]);
        pos--;
    }
}

void 
Node::sortRightward(int pos, Entry* arr, int len)
{
    int count = 0;
    mfence();
    while(pos + 1 < len && arr[pos + 1].key < arr[pos].key)
    {
        std::swap(arr[pos], arr[pos + 1]);
        count += sizeof(Entry);

        if(count >= CacheLineSize)
        {
            clflush(&(arr[pos]));
            count -= CacheLineSize;
        }
        ++pos;
    }
    if(pos < len - 1 && arr[pos + 1].key == arr[pos].key)
    {
        while(pos < len - 1 && arr[pos + 1].key == arr[pos].key && arr[pos + 1].version.second != 0)
        {
            std::swap(arr[pos], arr[pos + 1]);
            count += sizeof(Entry);

            if(count >= CacheLineSize)
            {
                clflush(&(arr[pos]));
                count -= CacheLineSize;
            }
            ++pos;
        }
    }

    if(count != 0)
        clflush(&(arr[pos]));
    mfence();
}


void 
Node::insert(int insertedKey, void* ptr, int& newKey, void* & new_Node_left, void* &new_Node_right)
{
    // std::cout << "node insert called" << std::endl;
    // std::cout << "in node insert is_leaf = " << is_leaf << std::endl;
    // for internal nodes
    // find the child node that the key could map to
    // pos of the selected ptr
    int pos = -1;
    for(int i = 0; i < num_entries; ++i)
    {
        // insert into live node
        if(entries[i].key >= insertedKey && entries[i].version.second == 0)
        {  
            pos = i;
            break;
        }
    }

    assert(pos != -1);

    assert(entries[pos].pointer != nullptr);
    void* newNodeLeft;
    void* newNodeRight;
    int splitKey;
    Node* child = reinterpret_cast<Node*> (entries[pos].pointer);
    child->insert(insertedKey, ptr, splitKey, newNodeLeft, newNodeRight);

    int tempArrLen = 0;
    Entry* tempArr;

    if(newNodeLeft == nullptr)
    {
        // no split is done at child's level, safe to return
        new_Node_left = nullptr;
        new_Node_right = nullptr;
        return;
    }
    else
    {
        assert(newNodeLeft != nullptr);
        assert(newNodeRight != nullptr);
        // first look for reusable entries
        int reusePosOne;
        int reusePosTwo;
        bool match = reuseEntriesTwo(reusePosOne, reusePosTwo);
        // newNodeLeft != nullptr && newNodeRight == nullptr
        entries[pos].version.second = btree->getVersion();

        // for debug
        // std::cout << "beforehand, in Node::insert set " << pos << "to " << btree->getVersion() << std::endl;

        if(match)
        {
            // // for debug
            // std::cout << std::endl << "node address = " << this << std::endl;
            // std::cout << "before reuse, in Node::insert reuse entries, num_live_entries = " << num_live_entries << std::endl;
            // for debug
            // for(int i = 0; i < num_entries; ++i)
            // {
            //     std::cout << "entries[" << i << "].key = " << entries[i].key << std::endl;
            //     std::cout << "entries[" << i << "].version.second = " << entries[i].version.second << std::endl;
            // }
            // std::cout << "reusePosOne = " << reusePosOne << std::endl;
            // std::cout << "reusePosTwo = " << reusePosTwo << std::endl;
            // std::cout << std::endl;


            // there are two reusable entries
            entries[reusePosOne].key = splitKey;
            entries[reusePosOne].pointer = newNodeLeft;
            entries[reusePosOne].version.first = btree->getVersion();
            entries[reusePosOne].version.second = 0;
            entries[reusePosOne].valid = true;

            entries[reusePosTwo].key = entries[pos].key;
            entries[reusePosTwo].pointer = newNodeRight;
            entries[reusePosTwo].version.first = btree->getVersion();
            entries[reusePosTwo].version.second = 0;
            entries[reusePosTwo].valid = true;

            int right_key = entries[pos].key;
            void* right_ptr = newNodeRight;

            // for debug
            // std::cout << "reusePosOne = " << reusePosOne << std::endl;
            // std::cout << "reusePosTwo = " << reusePosTwo << std::endl;

            if(reusePosOne == 0)
                sortRightward(reusePosOne, entries, num_entries);
            else if(reusePosOne == num_entries - 1)
                sortLeftward(reusePosOne, entries);
            else if(entries[reusePosOne].key >= entries[reusePosOne + 1].key)
                sortRightward(reusePosOne, entries, num_entries);
            else
                sortLeftward(reusePosOne, entries);

            if(entries[reusePosTwo - 1].key == right_key && entries[reusePosTwo - 1].pointer == right_ptr 
                && entries[reusePosTwo - 1].version.second == 0)
            {
                --reusePosTwo;
            }
            else
            {
                assert(entries[reusePosTwo].key == right_key);
                assert(entries[reusePosTwo].pointer == right_ptr);
                assert(entries[reusePosTwo].version.second == 0);
            }

            if(reusePosTwo == 0)
                sortRightward(reusePosTwo, entries, num_entries);
            else if(reusePosTwo == num_entries - 1)
                sortLeftward(reusePosTwo, entries);
            else if(entries[reusePosTwo].key >= entries[reusePosTwo + 1].key)
                sortRightward(reusePosTwo, entries, num_entries);
            else
                sortLeftward(reusePosTwo, entries);

            // num_entries does not change
            // num_live_entries inc by 1 (-1+2)
            setNumLiveEntries(num_live_entries + 1);
            // // for debug
            // std::cout << "node address = " << this << std::endl;
            // std::cout << "after reuse entries, num_live_entries = " << num_live_entries << std::endl;
            // for debug
            // for(int i = 0; i < num_entries; ++i)
            // {
            //     std::cout << "entries[" << i << "].key = " << entries[i].key << std::endl;
            //     std::cout << "entries[" << i << "].version.second = " << entries[i].version.second << std::endl;
            // }
            // std::cout << "node address = " << this << "done" << std::endl << std::endl;

            clflush(&(num_live_entries));
            clflush(&(entries[pos].version));
            mfence();
        }
        else if(num_entries <= BranchFactorInternal - 2)
        {
            // exactly two entries to hold the two new entries
            entries[num_entries].key = splitKey;
            entries[num_entries].pointer = newNodeLeft;
            entries[num_entries].version.first = btree->getVersion();
            entries[num_entries].version.second = 0;
            entries[num_entries].valid = true;

            entries[num_entries + 1].key = entries[pos].key;
            entries[num_entries + 1].pointer = newNodeRight;
            entries[num_entries + 1].version.first = btree->getVersion();
            entries[num_entries + 1].version.second = 0;
            entries[num_entries + 1].valid = true;

            sortLeftward(num_entries, entries);
            sortLeftward(num_entries + 1, entries);

            // num_live_entries inc by 1 (-1+2)
            setNumLiveEntries(num_live_entries + 1);

            // std::cout << "in Node::insert use vacant entries, num_live_entries = " << num_live_entries << std::endl;
            // num_entries inc by 2
            setNumEntries(num_entries + 2);
            clflush(&(num_live_entries));
            clflush(&(entries[pos].version));
            mfence();

            // // for debug
            // std::cout << "node address = " << this << std::endl;
            // for(int i = 0; i < num_entries; ++i)
            // {
            //     std::cout << "entries[" << i << "].key = " << entries[i].key << std::endl;
            //     std::cout << "entries[" << i << "].version.second = " << entries[i].version.second << std::endl;
            // }
            // std::cout << "node address = " << this << "done" << std::endl << std::endl;;
        }
        else
        {
            // split needed
            // construct temp array
            // only + 1 because -1 then +2
            tempArrLen = num_live_entries + 1;
            tempArr = new Entry[tempArrLen];
            int newPos = 0;
            for(int i = 0; i < num_entries; ++i)
            {
                if(entries[i].version.second == 0)
                {
                    tempArr[newPos] = entries[i];
                    //for debug
                    assert(entries[i].valid);

                    // inc ref counter
                    Node* n = reinterpret_cast<Node*> (entries[i].pointer);
                    n->incRefCounter();
                    ++newPos;
                }
            }

            // // for debug
            // std::cout << std::endl << "in Node::insert split needed = " << newPos << std::endl;
            // std::cout << "in Node::insert newPos = " << newPos << std::endl;
            // std::cout << "in Node::insert num_live_entries = " << num_live_entries << std::endl;
            // std::cout << "node address = " << this << std::endl;
            // // for debug
            // for(int i = 0; i < num_entries; ++i)
            // {
            //     std::cout << "entries[" << i << "].key = " << entries[i].key << std::endl;
            //     std::cout << "entries[" << i << "].version.second = " << entries[i].version.second << std::endl;
            // }
            // std::cout << "node address = " << this << "done" << std::endl << std::endl;;


            assert(newPos == num_live_entries - 1);

            tempArr[tempArrLen - 2].pointer = newNodeLeft;
            tempArr[tempArrLen - 2].key = splitKey;
            tempArr[tempArrLen - 2].version.first = btree->getVersion();
            tempArr[tempArrLen - 2].version.second = 0;
            tempArr[tempArrLen - 2].valid = true;

            tempArr[tempArrLen - 1].pointer = newNodeRight;
            tempArr[tempArrLen - 1].key = entries[pos].key;
            tempArr[tempArrLen - 1].version.first = btree->getVersion();
            tempArr[tempArrLen - 1].version.second = 0;
            tempArr[tempArrLen - 1].valid = true;

            sortLeftwardNoFlush(tempArrLen - 2, tempArr);
            sortLeftwardNoFlush(tempArrLen - 1, tempArr);
        }
    }

    // no split needed
    if(tempArrLen == 0)
    {
        // set return ptrs to null
        new_Node_left = nullptr;
        new_Node_right = nullptr;
        return;
    }

    // split needed
    // always returns 2 nodes, since if returning one node is enough, it can always be fixed by reusing entry w/o creating a new node

    // two new nodes needed (for split at this level, so cannot be leaf node)
    Node* leftChild = new Node;
    Node* rightChild = new Node;

    leftChild->setBtree(btree);
    rightChild->setBtree(btree);

    leftChild->alloc();
    rightChild->alloc();

    int numEntriesLeft = tempArrLen/2;
    int numEntriesRight = tempArrLen - tempArrLen/2;

    // left child
    int count = 0;
    for(int i = 0; i < numEntriesLeft; ++i)
    {
        leftChild->entries[i] = tempArr[i];
        count += sizeof(Entry);
        if(count >= CacheLineSize)
        {
            clflush(&(leftChild->entries[i]));
            count -= CacheLineSize;
        }
    }

    // set newKey
    newKey = leftChild->entries[numEntriesLeft - 1].key;

    // set key of last entry to infinite
    leftChild->entries[numEntriesLeft - 1].key = Infinite;

    clflush(&(leftChild->entries[numEntriesLeft - 1]));

    leftChild->setNumLiveEntries(numEntriesLeft);
    leftChild->setNumEntries(numEntriesLeft);
    // increment reference counting of new nodes
    leftChild->incRefCounter();
    // should be in the same cache line
    clflush(&(leftChild->num_entries));
    mfence();

    // right child
    count = 0;
    for(int i = 0; i < numEntriesRight; ++i)
    {
        rightChild->entries[i] = tempArr[i + numEntriesLeft];
        count += sizeof(Entry);
        if(count >= CacheLineSize)
        {
            clflush(&(rightChild->entries[i]));
            count -= CacheLineSize;
        }
    }
    rightChild->entries[numEntriesRight - 1].key = Infinite;
    clflush(&(rightChild->entries[numEntriesRight - 1]));

    rightChild->setNumLiveEntries(numEntriesRight);
    rightChild->setNumEntries(numEntriesRight);
    // increment reference counting of new nodes, should be in same cache line
    rightChild->incRefCounter();
    clflush(&(rightChild->num_entries));
    mfence();

    // // for debug
    // std::cout << "in Node::insert constructing two new nodes" << std::endl;
    // std::cout << "original node:" << std::endl;
    // this->print();
    // std::cout << "new nodes:" << std::endl;
    // rightChild->print();
    // leftChild->print();
    // std::cout << "all for Node::insert" << std::endl << std::endl;

    new_Node_left = reinterpret_cast<void*> (leftChild);
    new_Node_right = reinterpret_cast<void*> (rightChild);

    mfence();
    setEndVersion();
    mfence();

    delete[] tempArr;
}


void 
LeafNode::insert(int insertedKey, void* ptr, int& newKey, void* & new_Node_left, void* &new_Node_right)
{
    // std::cout << "in leaf node insert " << insertedKey << std::endl;

    // increase ref count for new record
    Record* r = reinterpret_cast<Record*> (ptr);
    r->incRefCounter();

    for(int i = 0; i < num_entries; ++i)
    {
        if(entries[i].key == insertedKey && entries[i].version.second == 0)
        {
            Record* r_list = reinterpret_cast<Record*> (entries[i].pointer);
            insertRecordList(r_list, reinterpret_cast<Record*>(ptr));
            new_Node_left = nullptr;
            new_Node_right = nullptr;
            return;
        }
    }

    int reusePos = reusableEntry();
    if(reusePos != -1)
    {
        entries[reusePos].key = insertedKey;
        entries[reusePos].version.first = btree->getVersion();
        entries[reusePos].version.second = 0;
        entries[reusePos].pointer = ptr;
        assert(entries[reusePos].valid);

        if(reusePos == num_entries - 1)
            sortLeftward(reusePos, entries);
        else if(reusePos == 0)
            sortRightward(reusePos, entries, num_entries);
        else if(entries[reusePos].key >= entries[reusePos + 1].key)
            sortRightward(reusePos, entries, num_entries);
        else
            sortLeftward(reusePos, entries);

        setNumLiveEntries(num_live_entries + 1);
        clflush(&num_live_entries);
        mfence();

        new_Node_left = nullptr;
        new_Node_left = nullptr;
    }
    else if(num_entries < BranchFactorLeaf)
    {
        // no reusable entry in the node
        // but there is vacant position
        entries[num_entries].key = insertedKey;
        entries[num_entries].version.first = btree->getVersion();
        entries[num_entries].version.second = 0;
        entries[num_entries].pointer = ptr;
        entries[num_entries].valid = true;

        sortLeftward(num_entries, entries);

        // increment number of entries
        setNumEntries(num_entries + 1);
        setNumLiveEntries(num_live_entries + 1);
        new_Node_left = nullptr;
        new_Node_right = nullptr;
        clflush(&num_live_entries);
        mfence();

        new_Node_left = nullptr;
        new_Node_left = nullptr;
    }
    else
    {
        // no reusable entry and the leaf node is full, needs splitting
        int tempArrLen = num_live_entries + 1;

        // needs two new nodes
        Entry* tempArr = new Entry[tempArrLen];
        int posNew = 0;
        for(int i = 0; i < num_entries; ++i)
        {
            if(entries[i].version.second == 0)
            {
                tempArr[posNew] = entries[i];
                // for debug
                assert(entries[i].valid);

                Record* r = reinterpret_cast<Record*> (entries[i].pointer);
                r->incRefCounter();
                ++posNew;
            }
        }

        // for debug
        // std::cout << "in leafNodeInsert posNew = " << posNew << std::endl;
        // std::cout << "in leafNodeInsert num_live_entries = " << num_live_entries << std::endl;
        assert(posNew == num_live_entries);

        tempArr[tempArrLen - 1].key = insertedKey;
        tempArr[tempArrLen - 1].pointer = ptr;
        tempArr[tempArrLen - 1].version.first = btree->getVersion();
        tempArr[tempArrLen - 1].version.second = 0;
        tempArr[tempArrLen - 1].valid = true;

        sortLeftwardNoFlush(tempArrLen - 1, tempArr);
        LeafNode* newNodeLeft = new LeafNode;
        LeafNode* newNodeRight = new LeafNode;
        newNodeLeft->alloc();
        newNodeRight->alloc();

        newNodeLeft->setBtree(btree);
        newNodeRight->setBtree(btree);

        int numEntriesLeft = tempArrLen/2;
        int numEntriesRight = tempArrLen - tempArrLen/2;

        // left node
        mfence();
        int count = 0;
        for(int i = 0; i < numEntriesLeft; ++i)
        {
            newNodeLeft->entries[i] = tempArr[i];
            count += sizeof(Entry);
            if(count >= CacheLineSize)
            {
                clflush(&(newNodeLeft->entries[i]));
                count -= CacheLineSize;
            }
        }
        if(count != 0)
            clflush(&(newNodeLeft->entries[numEntriesLeft - 1]));

        newNodeLeft->setNumLiveEntries(numEntriesLeft);
        newNodeLeft->setNumEntries(numEntriesLeft);
        newNodeLeft->incRefCounter();
        clflush(&(newNodeLeft->num_entries));
        // set nextNode
        newNodeLeft->setNextNode(newNodeRight);
        clflush(&(newNodeLeft->nextNode));
        mfence();

        // newKey
        newKey = newNodeLeft->entries[numEntriesLeft - 1].key;

        // right node
        count = 0;
        for(int i = 0; i < numEntriesRight; ++i)
        {
            newNodeRight->entries[i] = tempArr[i + numEntriesLeft];
            count += sizeof(Entry);
            if(count >= CacheLineSize)
            {
                clflush(&(newNodeRight->entries[i]));
                count -= CacheLineSize;
            }
        }
        if(count != 0)
            clflush(&(newNodeRight->entries[numEntriesRight - 1]));

        newNodeRight->setNumLiveEntries(numEntriesRight);
        newNodeRight->setNumEntries(numEntriesRight);
        newNodeRight->incRefCounter();
        clflush(&(newNodeRight->num_entries));

        // set next & prev
        newNodeRight->setNextNode(nextNode);
        newNodeRight->setPrevNode(newNodeLeft);

        newNodeLeft->setNextNode(newNodeRight);
        newNodeLeft->setPrevNode(prevNode);

        if(prevNode != nullptr)
        {
            prevNode->setNextNode(newNodeLeft);
            clflush(&(prevNode->nextNode));
        }

        clflush(&(newNodeRight->nextNode));
        clflush(&(newNodeLeft->nextNode));
        mfence();

        // // for debug
        // std::cout << "in LeafNode::insert constructing two new nodes" << std::endl;
        // std::cout << "original leaf:" << std::endl;
        // this->print();
        // std::cout << "new leaf:" << std::endl;
        // newNodeLeft->print();
        // newNodeRight->print();
        // std::cout << "all for LeafNode::insert" << std::endl << std::endl;

        delete[] tempArr;

        new_Node_left = reinterpret_cast<void*> (newNodeLeft);
        new_Node_right = reinterpret_cast<void*> (newNodeRight);

        setEndVersion();
        mfence();

        // increment btree's version
    }
    // std::cout << "pos = " << pos << std::endl;
    // for debug
    // std::cout << "leaf node insert done" << std::endl;
}

void
Node::balanceChilds(int pos_1, int pos_2)
{
    // for debug
    // std::cout << "in balanceChilds" << std::endl;

    Node* leftChild = reinterpret_cast<Node*> (entries[pos_1].pointer);
    Node* rightChild = reinterpret_cast<Node*> (entries[pos_2].pointer);

    // used to replace Infinite key value in left child
    int replaceKey = entries[pos_1].key;

    // for debug
    assert(leftChild != nullptr);
    assert(rightChild != nullptr);

    // construct a temp array for copy
    int numLiveEntriesLeft = leftChild->num_live_entries;
    int numLiveEntriesRight = rightChild->num_live_entries;

    int numEntriesLeft = leftChild->num_entries;
    int numEntriesRight = rightChild->num_entries;

    int size = numLiveEntriesLeft + numLiveEntriesRight;
    // size after balance
    int numEntriesLeftNew = size/2;
    int numEntriesRightNew = size - size/2;

    Entry* tempArr = new Entry[size];
    int newPos = 0;
    // copy from left child
    for(int i = 0; i < numEntriesLeft; ++i)
    {
        if(leftChild->entries[i].version.second == 0)
        {
            tempArr[newPos] = leftChild->entries[i];
            assert(leftChild->entries[i].valid);
            assert(leftChild->entries[i].pointer != nullptr);
            ++newPos;
        }
        // dec ref counter on old data
        else if(leftChild->is_leaf)
        {
            // for record
            Record* r = reinterpret_cast<Record*> (leftChild->entries[i].pointer);
            assert(r != nullptr);
            r->decRefCounter();
            if(r->isRefZero())
                deleteRecordList(r);
        }
        else
        {
            // for node
            Node* n = reinterpret_cast<Node*> (leftChild->entries[i].pointer);
            assert(n != nullptr);
            n->decRefCounter();
            if(n->isRefZero())
            {
                n->dealloc();
                delete n;
            }
        }
    }
    // for debug
    assert(newPos == numLiveEntriesLeft);
    // assert(tempArr[newPos].key == Infinite);

    // replace Infinite with actual value from parent, only for non-leaf node
    if(!leftChild->is_leaf)
        tempArr[numEntriesLeft - 1].key = replaceKey;

    // copy from left child
    for(int i = 0; i < numEntriesRight; ++i)
    {
        if(rightChild->entries[i].version.second == 0)
        {
            assert(rightChild->entries[i].valid);
            assert(rightChild->entries[i].pointer != nullptr);
            tempArr[newPos] = rightChild->entries[i];
            ++newPos;
        }
        else
        {
            // dec ref counter on old data
            if(rightChild->is_leaf)
            {
                // for record
                Record* r = reinterpret_cast<Record*> (rightChild->entries[i].pointer);
                assert(r != nullptr);
                r->decRefCounter();
                if(r->isRefZero())
                    deleteRecordList(r);
            }
            else
            {
                // for node
                Node* n = reinterpret_cast<Node*> (rightChild->entries[i].pointer);
                assert(n != nullptr);
                n->decRefCounter();
                if(n->isRefZero())
                {
                    n->dealloc();
                    delete n;
                }

            }
        }
    }
    // for debug
    assert(newPos == size);
    // assert(tempArr[newPos].key == Infinite);

    // std::cout << "copy from originial nodes in balance finishsed" << std::endl;

    Node* newNodeLeft;
    Node* newNodeRight;
    Entry* newParentEntryList;

    if(leftChild->is_leaf)
    {
        assert(rightChild->is_leaf);
        newParentEntryList = new Entry[BranchFactorLeaf];

        newNodeLeft = new LeafNode;
        newNodeRight = new LeafNode;
        newNodeLeft->alloc();
        newNodeRight->alloc();

        newNodeLeft->setBtree(btree);
        newNodeRight->setBtree(btree);
    }
    else
    {
        assert(!rightChild->is_leaf);
        newParentEntryList = new Entry[BranchFactorInternal];

        newNodeLeft = new Node;
        newNodeRight = new Node;
        newNodeLeft->alloc();
        newNodeRight->alloc();

        newNodeLeft->setBtree(btree);
        newNodeRight->setBtree(btree);
    }

    // construct left child
    mfence();
    int count = 0;
    for(int i = 0; i < numEntriesLeftNew; ++i)
    {
        newNodeLeft->entries[i] = tempArr[i];
        count += sizeof(Entry);
        if(count >= CacheLineSize)
        {
            clflush(&(newNodeLeft->entries[i]));
            count -= CacheLineSize;
        }
    }

    // new key to divide the two nodes
    int newSplitKey = newNodeLeft->entries[numEntriesLeftNew - 1].key;
    assert(newSplitKey != Infinite);

    // set the padding only for non-leaf node
    if(!leftChild->is_leaf)
        newNodeLeft->entries[numEntriesLeftNew - 1].key = Infinite;
    clflush(&(newNodeLeft->entries[numEntriesLeftNew - 1].key));

    newNodeLeft->setNumEntries(numEntriesLeftNew);
    newNodeLeft->setNumLiveEntries(numEntriesLeftNew);
    clflush(&(newNodeLeft->num_entries));

    // construct right child
    count = 0;
    for(int i = 0; i < numEntriesRightNew; ++i)
    {
        newNodeRight->entries[i] = tempArr[i + numEntriesLeftNew];
        count += sizeof(Entry);
        if(count >= CacheLineSize)
        {
            clflush(&(newNodeRight->entries[i]));
            count -= CacheLineSize;
        }
    }
    if(!rightChild->is_leaf)
        newNodeRight->entries[numEntriesRightNew - 1].key = Infinite;
    clflush(&(newNodeRight->entries[numEntriesRightNew - 1].key));

    newNodeRight->setNumEntries(numEntriesRightNew);
    newNodeRight->setNumLiveEntries(numEntriesRightNew);
    clflush(&(newNodeRight->num_entries));
    mfence();

    delete[] tempArr;

    // for debug
    // std::cout << "constructing new nodes completed in merge" << std::endl;

    int parentIdx = 0;
    count = 0;
    for(int i = 0; i < num_entries; ++i)
    {
        // for debug
        // assert(parentIdx < num_entries);
        // std::cout << "BranchFactorInternal =" << BranchFactorInternal << std::endl;
        // std::cout << num_entries << std::endl;
        // std::cout << "parentIdx = " << parentIdx << std::endl;

        if(i == pos_1)
        {
            // for debug
            // std::cout << "parentIdx = " << parentIdx << std::endl;
            // std::cout << "pos_1 = " << pos_1 << std::endl;
            // std::cout << "parentIdx = " << parentIdx << std::endl << std::endl;

            newParentEntryList[parentIdx].pointer = reinterpret_cast<void*> (newNodeLeft);
            newParentEntryList[parentIdx].key = newSplitKey;
            newParentEntryList[parentIdx].version.first = btree->getVersion();
            newParentEntryList[parentIdx].version.second = 0;
            newParentEntryList[parentIdx].valid = true;

            count += sizeof(Entry);
            ++parentIdx;
        }
        else if(i == pos_2)
        {
            // for debug
            // std::cout << "parentIdx = " << parentIdx << std::endl;
            // std::cout << "pos_2 = " << pos_2 << std::endl;
            // std::cout << "parentIdx = " << parentIdx << std::endl << std::endl;

            newParentEntryList[parentIdx].pointer = reinterpret_cast<void*> (newNodeRight);
            newParentEntryList[parentIdx].key = entries[i].key;
            newParentEntryList[parentIdx].version.first = btree->getVersion();
            newParentEntryList[parentIdx].version.second = 0;
            newParentEntryList[parentIdx].valid = true;

            count += sizeof(Entry);
            ++parentIdx;
        }
        else if(entries[i].valid && entries[i].version.second == 0)
        {
            // for debug
            // std::cout << "i = " << i << ", copy from originial entries" << std::endl;

            newParentEntryList[parentIdx] = entries[i];
            assert(entries[i].valid);
            count += sizeof(Entry);
            ++parentIdx;

            // std::cout << "copy from originial entries done" << std::endl;
        }
        else
        {
            Node* n = reinterpret_cast<Node*> (entries[i].pointer);
            n->decRefCounter();
            if(n->isRefZero())
            {
                n->dealloc();
                delete n;
            }
        }

        if(count >= CacheLineSize)
        {
            clflush(&(newParentEntryList[i]));
            count -= CacheLineSize;
        }
    }

    if(leftChild->is_leaf)
    {
        // no need to set nextNode here since no deletion
        for(int i = parentIdx + 1; i < BranchFactorLeaf; ++i)
        {
            count += sizeof(Entry);
            if(count >= CacheLineSize)
            {
                clflush(&(newParentEntryList[i]));
                count -= CacheLineSize;
            }
        }
        clflush(&(newParentEntryList[BranchFactorLeaf - 1]));
    }
    else
    {
        // child is internal node
        for(int i = parentIdx + 1; i < BranchFactorInternal; ++i)
        {
            count += sizeof(Entry);
            if(count >= CacheLineSize)
            {
                clflush(&(newParentEntryList[i]));
                count -= CacheLineSize;
            }
        }
        clflush(&(newParentEntryList[BranchFactorInternal - 1]));
    }

    // for debug
    // std::cout << "parent array update done" << std::endl;

    mfence();

    Entry* oldEntry = entries;
    entries = newParentEntryList;
    clflush(&entries);

    setNumEntries(parentIdx);
    setNumLiveEntries(parentIdx);
    clflush(&num_entries);
    mfence();
    delete[] oldEntry;
    // no need to set num_entries for current node

    // std::cout << "balance done" << std::endl;
    // for(int i = 0; i < num_entries; ++i)
    // {
    //     std::cout << "i:" << i << " valid " << entries[i].valid << " pointer:" << entries[i].pointer << " end version: " << entries[i].version.second << std::endl;
    // }
}

void
Node::mergeChilds(int pos_1, int pos_2)
{
    // std::cout << "in mergeChilds" << std::endl;

    // need to update num_entries for current node
    Node* leftChild = reinterpret_cast<Node*> (entries[pos_1].pointer);
    Node* rightChild = reinterpret_cast<Node*> (entries[pos_2].pointer);

    assert(leftChild != nullptr);
    assert(rightChild != nullptr);

    // used to replace Infinite key value in left child, for non-leaf node only
    int replaceKey = entries[pos_1].key;

    Node* newNode;
    Entry* newParentEntryList;
    int numLiveEntriesLeft = leftChild->num_live_entries;
    int numLiveEntriesRight = rightChild->num_live_entries;

    int numEntriesLeft = leftChild->num_entries;
    int numEntriesRight = rightChild->num_entries;

    int size = numLiveEntriesLeft + numLiveEntriesRight;
    mfence();
    if(leftChild->is_leaf)
    {
        assert(rightChild->is_leaf);
        newNode = new LeafNode;
        newNode->setBtree(btree);
        newNode->alloc();
        // set next & prev
        LeafNode* next = (dynamic_cast<LeafNode*> (rightChild))->nextNode;
        LeafNode* prev = (dynamic_cast<LeafNode*> (leftChild))->prevNode;
        (dynamic_cast<LeafNode*> (newNode))->setNextNode(next);
        (dynamic_cast<LeafNode*> (newNode))->setPrevNode(prev);
        LeafNode* l = dynamic_cast<LeafNode*> (newNode);
        clflush(&(l->nextNode));
        clflush(&(l->prevNode));
        // set the prev node's next ptr
        if(prev != nullptr)
        {
            prev->setNextNode(newNode);
            clflush(&(prev->nextNode));
        }

        newParentEntryList = new Entry[BranchFactorLeaf];
    }
    else
    {
        assert(!rightChild->is_leaf);
        newNode = new Node;
        newNode->alloc();
        newNode->setBtree(btree);
        newParentEntryList = new Entry[BranchFactorInternal];
    }

    int newPos = 0;
    int count = 0;
    for(int i = 0; i < numEntriesLeft; ++i)
    {
        // for debug
        assert(leftChild->entries[i].pointer != nullptr);
        if(leftChild->entries[i].version.second == 0)
        {
            newNode->entries[newPos] = leftChild->entries[i];
            assert(leftChild->entries[i].valid);
            ++newPos;
        }
        // dec ref counter on old data
        else if(leftChild->is_leaf)
        {
            // for record
            Record* r = reinterpret_cast<Record*> (leftChild->entries[i].pointer);
            r->decRefCounter();
            if(r->isRefZero())
                deleteRecordList(r);
        }
        else
        {
            // for node
            Node* n = reinterpret_cast<Node*> (leftChild->entries[i].pointer);
            n->decRefCounter();
            if(n->isRefZero())
            {
                n->dealloc();
                delete n;
            }
        }

        // for flush
        count += sizeof(Entry);
        if(count >= CacheLineSize)
        {
            // for debug
            // assert(&(newNode->entries[newPos]) != nullptr);

            clflush(&(newNode->entries[newPos]));
            count -= CacheLineSize;
        }
    }
    // set the last key to non-infinite for non-leaf node
    if(!leftChild->is_leaf)
        newNode->entries[numEntriesLeft - 1].key = replaceKey;

    // for debug
    assert(newPos == numLiveEntriesLeft);

    // std::cout << "after copying leftChild in merge" << std::endl;

    for(int i = 0; i < numEntriesRight; ++i)
    {
        if(rightChild->entries[i].version.second == 0)
        {
            assert(rightChild->entries[i].valid);
            newNode->entries[newPos] = rightChild->entries[i];
            ++newPos;
        }
        else
        {
            // dec ref counter on old data
            if(rightChild->is_leaf)
            {
                // for record
                Record* r = reinterpret_cast<Record*> (rightChild->entries[i].pointer);
                r->decRefCounter();
                if(r->isRefZero())
                    deleteRecordList(r);
            }
            else
            {
                // for node
                Node* n = reinterpret_cast<Node*> (rightChild->entries[i].pointer);
                n->decRefCounter();
                if(n->isRefZero())
                {
                    n->dealloc();
                    delete n;
                }

            }
        }

        count += sizeof(Entry);
        if(count >= CacheLineSize)
        {
            // for debug
            // assert(&(newNode->entries[newPos]) != nullptr);

            clflush(&(newNode->entries[newPos]));
            count -= CacheLineSize;
        }
    }

    if(count > 0)
    {
        // for debug
        // assert(&(newNode->entries[newPos - 1]) != nullptr);

        clflush(&(newNode->entries[newPos - 1]));
    }
    // for debug
    assert(newPos == size);

    // for debug
    // std::cout << "after copying to newNode in mergeChilds" << std::endl;

    newNode->setNumEntries(size);
    newNode->setNumLiveEntries(size);

    // for debug
    // assert(&(newNode->num_entries) != nullptr);

    clflush(&(newNode->num_entries));

    // update parent's entries arr
    int parentIdx = 0;
    count = 0;
    for(int i = 0; i < num_entries; ++i)
    {
        if(i == pos_1)
        {
            newParentEntryList[parentIdx].key = entries[pos_2].key;
            newParentEntryList[parentIdx].pointer = reinterpret_cast<void*> (newNode);
            newParentEntryList[parentIdx].version.first = btree->getVersion();
            newParentEntryList[parentIdx].version.second = 0;
            newParentEntryList[parentIdx].valid = true;

            ++parentIdx;
            count += sizeof(Entry);
        }
        else if(i == pos_2)
        {
            // dec the ref counter on the node
            Node* n = reinterpret_cast<Node*> (entries[i].pointer);
            n->decRefCounter();
            // no recursice dealloc, as is already done previously
            if(n->isRefZero())
                delete n;
        }
        else if(entries[i].version.second == 0)
        {
            newParentEntryList[parentIdx] = entries[i];
            assert(&(entries[i]));

            ++parentIdx;
            count += sizeof(Entry);
        }
        else
        {
            // dec the ref counter on the node
            Node* n = reinterpret_cast<Node*> (entries[i].pointer);
            n->decRefCounter();
            // no recursice dealloc, as is already done previously
            if(n->isRefZero())
            {
                n->dealloc();
                delete n;
            }
        }

        if(count >= CacheLineSize)
        {
            clflush(&(newParentEntryList[parentIdx - 1]));
            count -= CacheLineSize;
        }
    }
    // flush the pointer to the array
    Entry* oldEntry = entries;
    entries = newParentEntryList;
    clflush(&entries);
    setNumEntries(parentIdx);
    setNumLiveEntries(parentIdx);
    clflush(&num_entries);

    mfence();
    delete[] oldEntry;


    // std::cout << "done mergeChilds" << std::endl;
}

void 
Node::merge(int pos)
{
    // std::cout << "in merge" << std::endl;
    int pos_2 = pos;
    int pos_1 = findLiveEntryLeft(pos);
    if(pos_1 == -1)
    {
        pos_2 = findLiveEntryRight(pos);
        pos_1 = pos;
    }

    // no available nodes for merge
    // return and leave the work to upper level nodes
    if(pos_1 == -1 || pos_2 == -1)
        return;

    Node* leftChild = reinterpret_cast<Node*> (entries[pos_1].pointer);
    Node* rightChild = reinterpret_cast<Node*> (entries[pos_2].pointer);
    int numLiveEntriesLeft = leftChild->num_live_entries;
    int numLiveEntriesRight = rightChild->num_live_entries;

    bool isChildLeaf = false;
    if(leftChild->is_leaf)
        isChildLeaf = true;

    if(isChildLeaf)
    {
        if(numLiveEntriesLeft + numLiveEntriesRight <= BranchFactorLeaf)
            mergeChilds(pos_1, pos_2);
        else
            balanceChilds(pos_1, pos_2);
    }
    else
    {
        // plus 1 for key from parent (the current node)
        if(numLiveEntriesLeft + numLiveEntriesRight <= BranchFactorInternal)
            mergeChilds(pos_1, pos_2);
        else
            balanceChilds(pos_1, pos_2);
    }

    // std::cout << "in merge done" << std::endl;
}

bool 
Node::remove(int rmKey)
{
    // std::cout << "in Node remove, key = " << rmKey << std::endl;
    // find child node idx that could map to rmKey 
    int pos = -1;
    for(int i = 0; i < num_entries; ++i)
    {
        if(entries[i].key >= rmKey && entries[i].version.second == 0)
        {
            pos = i;
            break;
        }
    }

    if(pos != -1)
    {
        assert(entries[pos].pointer != nullptr);
        Node* child = reinterpret_cast<Node*> (entries[pos].pointer);
        // for debug
        assert(child != nullptr);
        //
        bool needsMerging = child->remove(rmKey);
        if(needsMerging)
            merge(pos);
    }

    // std::cout << "Node remove done, key = " << rmKey << std::endl;

    if(num_live_entries < BranchFactorInternal/4)
        return true;
    else
        return false;
}

bool 
LeafNode::remove(int rmKey)
{
    // std::cout << "in LeafNode remove, key = " << rmKey << std::endl;

    int idx;
    for(idx = 0; idx < num_entries; ++idx)
    {
        // delete live data
        if(entries[idx].key == rmKey && entries[idx].version.second == 0)
        {
            // for debug
            assert(entries[idx].valid);

            mfence();
            entries[idx].version.second = btree->getVersion();
            setNumLiveEntries(num_live_entries - 1);
            clflush(&num_live_entries);
            clflush(&(entries[idx].version));
            mfence();
            break;
        }
    }

    if(num_live_entries < BranchFactorLeaf/4)
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
        // std::cout << std::endl << "in leaf scan node address = " << ptr << std::endl;
        int num = ptr->num_entries;
        for(int i = 0; i < num; ++i)
        {
            // for debug
            // std::cout << "in leaf scan key = " << ptr->entries[i].key << std::endl;
            // std::cout << "in leaf scan version.second = " << ptr->entries[i].version.second << std::endl;
            // std::cout << "node ref count = " << ptr->refCounter << std::endl;
            if(ptr->entries[i].key >= lower && ptr->entries[i].key < upper && ptr->entries[i].version.second == 0)
            {
                Record* r = reinterpret_cast<Record*> (ptr->entries[i].pointer);
                // printRecordList(r);
            }
            else if(ptr->entries[i].key >= upper && ptr->entries[i].version.second == 0)
                return;
        }
        // assert(ptr != ptr->nextNode);
        ptr = ptr->nextNode;
    }
}

void
LeafNode::scan(int key)
{
    for(int i = 0; i < num_entries; ++i)
    {
        if(entries[i].key == key && entries[i].version.second == 0)
        {
            Record* r = reinterpret_cast<Record*> (entries[i].pointer);
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
    void* new_Node_left;
    void* new_Node_right;
    incVersion();
    // root->insert(key, ptr, newKey, new_Node);
    root->insert(key, ptr, newKey, new_Node_left, new_Node_right);
    // std::cout << "after root insert root->isLeaf" << std::endl;

    if(new_Node_left != nullptr)
    {
        assert(new_Node_right != nullptr);
        // std::cout << "in make a root" << std::endl;
        // make a new root
        mfence();
        Node* newRoot = new Node;
        newRoot->setBtree(this);
        newRoot->alloc();

        newRoot->entries[0].pointer = root;
        newRoot->entries[0].key = newKey;
        newRoot->entries[0].version.first = rootStartVersion;
        newRoot->entries[0].version.second = getVersion();
        newRoot->entries[0].valid = true;

        newRoot->entries[1].pointer = new_Node_left;
        newRoot->entries[1].key = newKey;
        newRoot->entries[1].version.first = getVersion();
        newRoot->entries[1].version.second = 0;
        newRoot->entries[1].valid = true;


        newRoot->entries[2].pointer = new_Node_right;
        newRoot->entries[2].key = Infinite;
        newRoot->entries[2].version.first = getVersion();
        newRoot->entries[2].version.second = 0;
        newRoot->entries[2].valid = true;

        newRoot->setNumEntries(3);
        newRoot->setNumLiveEntries(2);

        clflush(&(newRoot->entries[0]));
        clflush(&(newRoot->num_entries));

        root = newRoot;
        clflush(&root);
    }

    rootStartVersion = getVersion();
    
    clflush(&rootStartVersion);

    flushVersion();
}

void
BTree::remove(int key)
{
    incVersion();
    root->remove(key);
    if(root->num_live_entries == 1 && !root->is_leaf)
    {
        int pos = -1;
        bool match = false;
        for(int i =0; i < root->num_entries; ++i)
        {
            if(root->entries[i].version.second == 0)
            {
                assert(match == false);
                pos = i;
                match = true;
                rootStartVersion = root->entries[i].version.first;
            }
            else
            {
                Node* n = reinterpret_cast<Node*> (root->entries[i].pointer);
                n->decRefCounter();
                if(n->isRefZero())
                {
                    n->dealloc();
                    delete n;
                }
            }
        }
        assert(pos != -1);
        Node* newRoot = reinterpret_cast<Node*> (root->entries[pos].pointer);
        mfence();
        Node* oriRoot = root;
        root = newRoot;
        clflush(&root);
        clflush(&rootStartVersion);
        mfence();

        delete oriRoot;
    }
    flushVersion();
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
    // for(int i = lower; i < upper; ++i)
    // {
    //     // std::cout << "scanning " << i << std::endl;
    //     LeafNode* n = dynamic_cast<LeafNode*>(root->lookUp(i));
    //     n->scan(i);
    // }

    LeafNode* n = dynamic_cast<LeafNode*>(root->lookUp(lower));
    n->scan(lower, upper);
}