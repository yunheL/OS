#include "bplus_tree.h"
#include "time.h"
#include "stdint.h"

#define BILLION 1000000000L

/*
 * Function:  duplicate
 * --------------------
 *  Checks if array contains duplicates
 *
 *  arr: concerned array
 *  num_keys: number of keys in the array
 *  pos: pointer to an int which stores the index at which duplicate occurs
 *
 *  returns: 1 if duplicates present
 *           0 otherwise
 *           
 */

void
mem_flush(volatile void *p)
{
  asm volatile ("clflush (%0)" :: "r"(p));
  asm volatile ("mfence");
}
void
cflush(volatile void *p)
{
    asm volatile ("clflush (%0)" :: "r"(p));
}

int write_journal(node* root,
          node* n,
          int key,
          void* left,
          void*right,         
          int split,
          int crash,
          int del);

int
duplicate(int* arr, int num_keys, int* pos){
    
    /* Returns 1 if there is a duplicate in arr
       0 otherwise 
       Args: arr 
    */    
    int i;
    for(i=0; i<num_keys-1; i++){
        if(*(arr+i) == *(arr+i+1)){
            *pos = i;
            return 1;
        }
    }    
    return 0;
}

/*
    Find if a key from a lower node needs to be changed in the parent
    key: the key value deleted in the lower node
    n: the lower node
    new_lkey: the new leftmost key to replace the deleted key with
*/
void
check_parent_key(int key, node* n, int new_lkey){
    int i;
    int pos;
    bool found = false;
    node* parent = n->parent;
    int *arr = parent->keys;
    for(i=0; i < parent->num_keys; i++){
        if(key == *(arr + i)){
            pos = i;
            found = true;
            break;
        }
    }
    if(found){
        *(arr+pos) = new_lkey;
    }
    else {
        if(parent->parent != NULL){
            //recursive look up tree to root
            check_parent_key(key,parent,new_lkey);
        }
    }
    
    
    
}
/*
    Removes the child from the parents pointers and checks for enough childern and borrows/merges if not
    n: node of the parent
    lkey: left most key of the dead child
*/
void
remove_child(node* n, int lkey){
    int i,pos,new_lkey,size;
    int *arr = n->keys;
    pos = n->num_keys;
    if(duplicate(arr,n->num_keys,&pos)){
        for(i=pos; i<n->num_keys-1;i++){
            *(arr+i) = *(arr+i+1);
            *(n->pointers+i+1) = *(n->pointers+i+2);
        }
        n->num_keys--;
    }
    //check to see if you have already deleted key
    else{
        pos = 0;
        for(i=0; i < n->num_keys; i++){
            if(lkey == *(arr+i)){
                pos = i;
                break;
            }
        }
        //moving everything over
        i = pos;
        if(n->log->consistent == 3){
            //do nothing you have already decremented the counter
        }
        else{
           while(i < n->num_keys){
                *(arr+i) = *(arr+i+1);
                *(n->pointers+i+1) = *(n->pointers+i+2);
                i++;
            }
            n->num_keys--; 
            mem_flush(n->log);
            n->log->consistent = 3;
        }
        
    }

    //check if violation of size
    
    size = n->num_keys;
    new_lkey = *arr;
    node* parent = n->parent;
    if(size < (order-1)/2){
        if(parent != NULL){
            int *parent_arr = parent->keys;
            if(new_lkey < *parent_arr){
                get_right(n);
            }
            else {
                get_left(n,new_lkey);   
            }
            
        }
        else if(size == 0){
            //root is 0
            delete_node(parent);
        }
        
    }
    
}

/*
    Moves the keys and values from the right node over to the left_node, frees the right node and removes the pointer for the right node in the parent
    checks to make sure parent has enough pointers and if not performs borrowing/merging again
    
*/
void
merge(node* left_node, node* right_node){
    int i,right_lkey;
    int *right_arr = right_node->keys;
    int *left_arr = left_node->keys;
    //node* right_temp = right_node;
    //node* left_temp = left_node;
    right_lkey = *right_arr;
    node* parent = right_node->parent;
    for(i=0; i < right_node->num_keys; i++){
        *(left_arr+left_node->num_keys + i) = *(right_arr + i);
        *(left_node->pointers+left_node->num_keys+i+1) = *(right_node->pointers + i);
    }
    left_node->num_keys = right_node->num_keys + left_node->num_keys;
    mem_flush(left_node->log);
    left_node->log->num_keys = left_node->num_keys;
    //remove right node from parents pointers
    remove_child(parent,right_lkey);
    //delete the right node
    delete_node(right_node);
}

/*
    Checks the parent of node n for left neighbor of n and returns the value of the highest value extra key
        if no extra key then calls function merge_left to merge the two neighbors
    n: node that needs a key
    lkey: leftmost key of node n
    don't really need to return anything since key appending happens here
    
*/
void
get_left(node* n, int lkey){
    int i,left,size,take_key;
    node* parent = n->parent;
    int *arr = parent->keys;
    int *child_arr = n->keys;
    void* value;
    //get the position of the left neighbor
    for(i=0; i < parent->num_keys; i++){
            //children nodes are the values of *(parent->pointers)
        if(lkey == *(arr + i)){
            left = i;
            /*//check that the right neighbor (node n) is actually correct
            if(*(parent->pointers+i+1) == n){
                printf("Got the right neighbor\n");
            }else {
                printf("Got the WRONG neighbor\n");
            }*/
            break;
        }
    }
    bool taken=false;
    node* left_node = *(parent->pointers+left);
    //need to make sure you haven't already take a key from left
    if(n->log->num_keys ==(order-1)/2){
        taken = true;
        
    }
    if(taken){
        size = ((order-1)/2) + 1;
    }
    else {
        size = left_node->num_keys;
    }

    int *left_arr = left_node->keys;
    if((size == (order-1)/2)&&(!taken)){
        //cannot take a key away, must merge
        merge(left_node,n);
    }
    else {
        //check to see if a merge already occurred
        if(*(left_arr+(left_node->num_keys-n->num_keys)) == *(n->keys)){
            left_node->num_keys--;
            merge(left_node,n);
        }
        else {
            //get the highest key in the left node and its associated value
            take_key = *(left_arr+(left_node->num_keys-1));
            value = left_node->pointers[(left_node->num_keys-1)];

            int pos = n->num_keys;
            if(duplicate(child_arr,n->num_keys,&pos)){
                for(i=pos-1; i>=0; i--){
                    *(child_arr+i+1) = *(child_arr+i);
                    *(n->pointers+i+2) = *(n->pointers+i + 1);
                }
                *(child_arr) = take_key;
                n->pointers[0] = value;
                n->num_keys++;
            }
            else if(*(child_arr) != take_key){
                for(i=n->num_keys-1; i>=0; i--){
                    *(child_arr+i+1) = *(child_arr+i);
                    *(n->pointers+i+2) = *(n->pointers+i + 1);
                } 
                n->num_keys++;
                n->pointers[0] = value;
                *(child_arr) = take_key;
                
                

            }
        }
        
        //decrement the number of keys in the neighbor
        if(n->log->num_keys != n->num_keys){
            left_node->num_keys--;
        }
        cflush(n->log);
        cflush(left_node->log);
        n->log->num_keys = n->num_keys; 
        left_node->log->num_keys = left_node->num_keys;
        
    }
    
}
/*
    Checks to see if the right neighbor has a spare key and if so takes the lowest value key
    get_right is a bit easier than left since you only borrow from right if the node is already farthest left
    n: node that needs a key
        
*/

void
get_right(node* n){
    int i,size,take_key,new_lkey;
    node* parent = n->parent;
    node* right_node = *(parent->pointers+1);
    bool taken=false;
    if(n->log->num_keys == (order-1)/2){
        taken = true;
    }
    if(taken){
        size = ((order-1)/2) + 1;
    }
    else {
        size = right_node->num_keys;
    }
    
    int *right_arr = right_node->keys;
    int *child_arr = n->keys;
    void* value;
    if(size == (order-1)/2){
        merge(n,right_node);
    }else {
        //get the lowest key in the right node and its associated value
        take_key = *(right_arr);
        value = right_node->pointers[0];



        //update parent with new left key
        new_lkey = *(right_arr+1);
        check_parent_key(take_key,right_node,new_lkey);
         //add the key and value in the original node
        *(child_arr+n->num_keys) = take_key;
        n->pointers[n->num_keys+1] = value;
        n->num_keys++;
        mem_flush(n->log);
        n->log->num_keys = n->num_keys;
        right_node->num_keys--;
        mem_flush(right_node->log);
        right_node->log->num_keys = right_node->num_keys;


        //node* temp = *(right_node->pointers);
        //printf("value taken first key %d\n",*(temp->keys));
        //shift keys and values over to the right
        i = 0;
        int pos=right_node->num_keys;
        if(duplicate(right_arr,right_node->num_keys,&pos)){
            for(i=pos; i<right_node->num_keys-1;i++){
                *(right_arr+i) = *(right_arr+i+1);
                *(right_node->pointers+i) = *(right_node->pointers+i+1);
            }
            right_node->num_keys--;
           	mem_flush(right_node->log);
            right_node->log->num_keys = right_node->num_keys;

        }
        else{
            while(i<right_node->num_keys+1){
                *(right_arr+i) = *(right_arr+i+1);
                *(right_node->pointers+i) = *(right_node->pointers+i+1);
                i++;
            }
            right_node->log->num_keys = right_node->num_keys;
	    cflush(right_node->log);
        }       
        
        
        

    }
    
}

void
delete_from_node_hard(node* leaf, int key){

    int i,pos=leaf->num_keys;
    int *arr = leaf->keys;
    //delete key and move things over
    if(duplicate(arr,leaf->num_keys,&pos)){
        for(i=pos; i<leaf->num_keys-1;i++){
            *(arr+i) = *(arr+i+1);
            *(leaf->pointers+i+1) = *(leaf->pointers+i+2);
        }
        leaf->num_keys--;
    }
    else {
        pos = 0;
        for(i=0; i < leaf->num_keys; i++){
            // Found spot i should delete from
            if(key == *(arr + i)){
                pos = i;
                break;
            }
        }

        i = pos;
	//        printf("pos: %d\n",pos);
        if(leaf->log->consistent == 3){
            //do nothing you have already decremented the counter
        }
        else {
           while(i<leaf->num_keys){
                *(arr+i) = *(arr+i+1);
                *(leaf->pointers+i+1) = *(leaf->pointers+i+2);
                i++;
            }
            leaf->num_keys--;  
            mem_flush(leaf->log);
            leaf->log->consistent = 3;   
        }
        
        
        
    }
    if(leaf->num_keys == 0){
        if(leaf->parent == NULL){
	    //  printf("DELETE TREE\n");
            delete_node(leaf);
        }
        else {
            remove_child(leaf->parent,key);
        }
    }
    else{
        int new_lkey = *arr;
        //handle left edge case
        if(pos == 0){
            if(leaf->parent != NULL){
                //need to update parent 
                check_parent_key(key,leaf,new_lkey);
            }
        
        }
        if(leaf->parent != NULL){
            int *parent_arr = leaf->parent->keys;
            //check if violating size amount
            if(leaf->num_keys < (order-1)/2){
                if(leaf->parent != NULL){
                    //check to see if this leaf is already farthest left
                    if(new_lkey < *parent_arr){
                        
                        get_right(leaf);
                    }
                    else {
                        get_left(leaf,new_lkey);    
                    }
            
                }
        
            }  
        }
 
    }

}

void
delete_from_node_simple(node* leaf,int key){
    int *arr = leaf->keys;
    int i, pos=leaf->num_keys;

    if(duplicate(arr,leaf->num_keys,&pos)){
        for(i=pos; i<leaf->num_keys-1;i++){
            *(arr+i) = *(arr+i+1);
            *(leaf->pointers+i+1) = *(leaf->pointers+i+2);
        }
    }
    else{
        for(i=0; i < leaf->num_keys; i++){
            // Found spot i should delete from
            if(key == *(arr + i)){
                pos = i;
                break;
            }
        }
        i = pos;
        while(i<leaf->num_keys){
            *(arr+i) = *(arr+i+1);
            *(leaf->pointers+i+1) = *(leaf->pointers+i+2);
            i++;
        }
    }
    
    leaf->num_keys--;
    int new_lkey = *arr;
    if(pos == 0){
            if(leaf->parent != NULL){
                check_parent_key(key,leaf,new_lkey);

            }
        
    }

}
int
check_consistency_delete_simple(node* n){
    log_* l= n->log;
    int last_delete = l->key,i;
    int *arr = n->keys;
    if(l->consistent== 2){
        for(i=0; i<n->num_keys; i++){
            if(last_delete == *(arr+i)){
                return 1;
                break;
            }
        }
        if(l->num_keys != n->num_keys) return 2;
    }
    return 0;
}

node*
make_consistent_delete_hard(node* n){

    int i;
    bool found_key;
    int *arr = n->keys;
    if(n->log->split = 0 || n->log->consistent == 1 || n->log->consistent == 0){
        n->log->consistent = 0;
        mem_flush(n->log);
        n->log->key = -1;
        n->log->split = 0;
        return n;
    }
    //check to see if key still in node
    for(i=0; i < n->num_keys; i++){
            // Found spot i should delete from
            if(n->log->key == *(arr + i)){
                found_key = true;
                break;
            }
    }
    //key still there
    if(found_key){
        delete_from_node_hard(n,n->log->key);
    }
    //need to check for borrowing/merging making tree wrong
    else {
        delete_from_node_hard(n,n->log->key);
    }
    

}

/*
    just remove the key from the node
    n: node
    flag: 0 for everything okay
        1 for need to delete key
        2 for metadata out of sync
*/
node*
make_consistent_delete_simple(node* n, int flag){
    if(flag == 0 || n->log->consistent == 1 || n->log->consistent == 0){
        n->log->consistent = 0;
        mem_flush(n->log);
        n->log->key = -1;
        return n;
    }

    //key needs to be deleted
    if(flag == 1){
       delete_from_node_simple(n,n->log->key);
    }
    n->log->num_keys = n->num_keys;
    n->log->consistent = 0;
    cflush(n->log);
    return n;
}

/*
  Finds the key specified in a leaf node,
  deletes the key from the leaf and then follows the following steps:
    1. If leaf node has enough keys just update parent recursive to root
    (Not enough keys)
    2. Look to left neighbor to find extra key
    3. merge with left neighbor if neighbor has minimum keys
    4. look to right neighbor to find extra key
    5. merge with right neighbor if neighbor has minimum keys
    6. After merging need to make sure that parent node has enough children, otherwise repeat steps 2 or 4 and try and takeneighbor parent's leaf
    7. If can't take leaf then repeat steps 3 or 5 to merge parent (internal) nodes
    returns 1 on failure, 0 otherwise
*/
int
delete_key(node* root, int key, int crash){
    int i,pos, new_key, cousin;
    bool found=false;
    //find the leaf with the given key
    node* leaf = find_leaf(root, key, false);
    int *arr = leaf->keys;
    //check to see if there is the key in the leaf returned
    if(leaf->num_keys == 0){
        return 1;
    }
    for(i=0; i<leaf->num_keys; i++){
        if(key == *(arr + i)){
            found = true;
            break;
        }
    }
    if(!found){
        return 1;
    }

    


    //simple no borrowing/merging
    if(leaf->num_keys > (order-1)/2){
        write_journal(root,
        leaf,
        key,
        NULL,
        NULL,
        0,
        crash,
        1); //delete
        make_consistent_delete_simple(leaf,1); 
        leaf->log->consistent = 0;
        cflush(leaf->log);
    }
    else{
        write_journal(root,
        leaf,
        key,
        NULL,
        NULL,
        1,
        crash,
        1); //delete
        make_consistent_delete_hard(leaf);
        if(leaf->log != NULL){
            leaf->log->consistent = 0;
            cflush(leaf->log);
        }
        
    }
    

    return 0;
}

/*
 * Function: insert_into_node_simple
 * ----------------------------------- 
 * Given leave node, inserts a key
 * and value into a leaf node.  Simple since we assume the node has
 * space available and no splitting need be done
 *
 *  n: concerned leaf node
 *  key: key to be inserted
 *  value: value associated with the key
 *
 *  returns: 0[doesn't mean anything-could've made it void]
 *           
 */
int
insert_into_node_simple(node *n,			 
                        int key,
                        void* value){

    int *arr= n->keys;    
    int i, pos=n->num_keys;
    // Check for duplicates first:
    // if there is a duplicate then I need to be careful
    // otherwise feel free to do bottom bit
    // I won't even test this: Assume it works :) Yayy academia
    if(duplicate(arr, n->num_keys, &pos)){
	for(i=pos; i>0;i--){

	    if(key < *(arr+i)){
		*(arr+i) = *(arr+i-1);
	    }
	    else{
		break;
	    }
	}
	*(arr+i+1) = key;        
    }
    else{	
        for(i=0; i < n->num_keys; i++){
            // Found spot i should insert into
            if(key < *(arr + i)){
                pos = i;
                break;
            }
        }
        // move things over
        for(i=n->num_keys-1; i>=pos; i--){
            // keys and values
            *(arr+i+1) = *(arr+i);
            // There's another spot at the end for these guys
            *(n->pointers+i+2) = *(n->pointers+i + 1);
        }
        *(arr+pos) = key;
        n->pointers[pos] = value;        
    }
    // Increment number of keys
    n->num_keys++;
    return 0;
}

/*
 * Function: check_consistency_simple
 * ----------------------------------- 
 * Given leave node, check if it is consistent with it's journal
 * [simple since no splits involved]
 *
 *  n: concerned leaf node
 *
 *  returns: 0 on success
 *           1 if we haven't inserted the key
 *             into the node still
 *           2 if everything else is fine but meta data mismatch
 *           
 */
int
check_consistency_simple(node* n){
    
    log_* l= n->log;
    int last_insert = l->key, i, flag=0;

    //Journal was written to fine
    //Get the last key 
    if(l->consistent == 2){        
        // I've found the key: it's there The key is the last thng
        // inserted so everything else must be in order
        for(i=0; i<n->num_keys; i++){
            if(*(n->keys+i) == last_insert){
                flag = 1;
                break;
            }
        }
	
        // This could mean we've shifted some stuff over
        // or not shifted anything over.
	// We could just insert the god damn key again
        if(flag == 0) return 1;
        
        // num keys don't match up: but everything else is sorted
        // it has to be: since num_keys is the last thing updated
        // and it won't get here if the key wasn't found.
        if(l->num_keys != n->num_keys) return 2;        
    }
    // Node is consitent but the journal value wasn't changed!
    return 0;
}

/*
 * Function: insert_into_parent_simple
 * -------------------------------
 *  Parent has spacce, just push stuff into it
 *
 *  parent: concerned parent node
 *  key: The key that will be added to this parent
 *  left_ptr: the left child associated with the key
 *  right_ptr: the right child associated with the key
 *
 *  returns: root node of updated tree
 *           
 */
void
insert_into_parent_simple(node* parent,
                          int key,
                          node* left_ptr,
                          node* right_ptr){

    // I know there won't be overflow
    int i;
    for(i=0; i< parent->num_keys; i++){
        if(key < *(parent->keys + i)) break;        
    }
    
    int pos=i;    
    for(i=parent->num_keys-1; i>=pos; i--){
        *(parent->keys+i+1) = *(parent->keys+i);
        *(parent->pointers+i+2) = *(parent->pointers+i+1);        
    }

    *(parent->keys+ pos) = key;
    
    parent->num_keys++;
    
    *(parent->pointers+pos) = left_ptr;    
    *(parent->pointers+pos+1) = right_ptr;

    
    left_ptr->parent = parent;
    right_ptr->parent = parent;

}


/*
 * Function: make_consistent_simple
 * ----------------------------------- 
 * Given leave node, make it
 * consistent with journal[simple since no splits involved]
 * 
 *  n: concerned leaf node
 *
 *  returns: consistent leaf node
 *           
 */
node* 
make_consistent_simple(node* n, int flag){

    // Either everythng is consistent or journal is shit,
    // eitherways I don't want to touch the Bplus tree
    // Reset journal and get the hell out.
    
    if(flag == 0 || n->log->consistent == 1 || n->log->consistent == 0){
	n->log->consistent = 0;
	cflush(n->log);
	n->log->key = -1;
        return n;
    }
    
    // key needs to be inserted: inserting into leaft node guaranteed
    if(flag == 1 ){
	// Journal was inconsistent and the key was cleared
	// I just made the journal 0 but ignore all data in their
	if(n->log->key == -1)
	    return n;
		
	if (n->is_leaf){
	    insert_into_node_simple(n,			 
				    n->log->key,
				    n->log->left);
	}
	else{
	    insert_into_parent_simple(n,
				      n->log->key,
				      n->log->left,
				      n->log->right);
	}
    }
    
    // this has to happen regardless
    n->num_keys = n->log->num_keys;
    n->log->consistent = 0;
    cflush(n->log);
    return n;
}

/*
 * Function: split
 * -------------------------------
 *  Split leaf node into two pieces
 *
 *  original: Node to be split
 *  left : blank new leaf node which is about to be left child associated with key
 *  right: blank new leaf node which is about to be right child associated with key
 *
 *  returns: root node of updated tree
 *           
 */
void
split(node* original, node** left, node** right){

    int pending_key = original->log->key;
    // it'll point to a record
    record* pending_value = original->log->left; 

    int mid_index = order/2, i, pos;
    
    // JUST USE THE STACK: it' cleaner and they're of small size anyway
    int buffer[sizeof(int)*order];
    void* value_buffer[sizeof(void*)*order];
    
    for(i=0; i < order - 1; i++){        
        if(pending_key > *(original->keys+i)){
            buffer[i] = original->keys[i];
            value_buffer[i] = (record*)original->pointers[i];               
        }        
        else break;

    }
    
    buffer[i] = pending_key;    
    value_buffer[i] = pending_value;     
    i++;
    
    while(i < order){
        buffer[i] = original->keys[+i-1];
        i++;
    }
    
    node* lft = *left;
    node* rght = *right;
    
    for(i=0; i< mid_index;i++){
        *(lft->keys+i) = *(buffer+i);
        *(lft->pointers + i) = *(value_buffer+i);        
    }

    for(i=0; i<order - mid_index;i++){
        *(rght->keys+i) = *(buffer +i + mid_index);
        *(rght->pointers + i) = *(value_buffer+i+ mid_index);        
    }
    
    // Fix the pointers problem for the leaf nodes
    *(lft->pointers + mid_index+1) =  *(rght->pointers);
    
    // META DATA
    lft->num_keys = mid_index;
    rght->num_keys = order - mid_index;
    
}


/*
 * Function: parent_split
 * -------------------------------
 *  Parent node that needs to be split
 *
 *  root: root of the tree
 *  n: node to which I wish to add
 *  key: The key that will be added to this parent
 *  child_left: the left child associated with the key
 *  child_right: the right child associated with the key
 *
 *  returns: root node of updated tree
 *           
 */
node*
parent_split(node* root,
             node* n,
             int key,
             node* child_left,
             node* child_right,
	     int crash){
  
    // This is only called if the parent is full
    // so I know I have to go up

    node* parent = n->parent; int change_flag=0;
    
    // I need to make node or
    if(parent == NULL){
        parent = make_node();
	change_flag = 1;

    }

    //-----------------------------------------------------------------
    //just split up node into 2, and have node point to the right guys
    //-----------------------------------------------------------------
    int i, pos=order-1, mid_index = (order-1)/2 ;
    node* left = make_node();
    node* right = make_node();
        
    //==========================COPYING INTO BUFFER===================
    int key_buffer[sizeof(int)*order];
    void* value_buffer[sizeof(void)*order+1];


    for(i=0; i<n->num_keys; i++){
        if(key < *(n->keys+i)){
            pos = i;
            break;
        }
        else{
            *(key_buffer+i) = *(n->keys+i);
            *(value_buffer+i) = *(n->pointers+i);
        }

    }

    // push everything to the side
    for(i=order-1; i>pos; i--){
        *(key_buffer+i) = *(n->keys+i-1);
        *(value_buffer+i+1) = *(n->pointers+i);
    }

    *(key_buffer+pos) = key;
    *(value_buffer+pos) = child_left;
    
    
    *(value_buffer+pos+1) = child_right;
     
    //===========================COPY END=============================

    node* temp;
    
    // LEFT
    // Doesn't take mid index but gets the ptr
    for(i=0; i< mid_index; i++){
        *(left->keys+i) = *(key_buffer+i);
        *(left->pointers+i) = *(value_buffer+i);	
	node* temp_child = (node*) *(left->pointers+i);
	temp_child->parent = left;
	
        temp = (node*)left->pointers[i];
    }    
    // The ptr under middle should go to left
    *(left->pointers+i) = *(value_buffer+i);
    temp = (node*)left->pointers[i];

    node* temp_child = (node*) *(left->pointers+i);
    temp_child->parent = left;
    
    left->num_keys = mid_index;

    // RIGHT
    int j=0;
    for(i=mid_index+1; i < order; i++,j++){
        *(right->keys+j) = *(key_buffer+i);
        *(right->pointers+j) = *(value_buffer+i);
        temp = (node*)right->pointers[j];
	temp->parent = right;
        
    }    
    // the last guy
    *(right->pointers+j) = *(value_buffer+i);
    temp = (node*)right->pointers[j];
    temp->parent = right;
    
    right->num_keys = order - mid_index -1;
    int middle_guy = *(key_buffer + mid_index);


    //*******************DANGER******************************************************
    if(crash == 13){
	//printf("%sSystem crashed in the middle of split parent.%s\n", KRED, KWHT);    
	return root;
    }    
    //********************************************************************************
        
    // NEED TO POINT PARENT TO THE RIGHT GUY
    if(child_left->keys[0] > middle_guy)
	child_left->parent = right;
    else
	child_left->parent = left;


    if(child_right->keys[0] > middle_guy)
	child_right->parent = right;
    else
	child_right->parent = left;
    
    //*******************DANGER******************************************************
    if(crash == 14){
	//printf("%sSplit parent node not yet complete but system crashed.%s\n", KRED, KWHT);    
	return root;
    }    
    //********************************************************************************
    
    //---------------SPLIT PARENT COMPLETE-----------------------------------------    
    if(parent->num_keys < order -1){
        //push the middle guy into parent
        insert_into_parent_simple(parent, middle_guy, left, right);        

    }
    else{
        root = parent_split(root, parent, middle_guy, left, right, crash);
    }

    // Memory leak
    /* delete_node(n); */

    // The last thing we ever update is the root i.e the tree itself. It fails anywhere
    // before this it won't matter.    
    if(change_flag)
	root = parent;
    
    return root;        
}

/*
 * Function: make_consistent_split
 * -------------------------------
 *  Node is pending a split, make it consistent with its journal
 *
 *  root: root of the tree
 *  node: node to which I wish to add
 *
 *  returns: root node of updated tree
 *           
 */
node*
make_consistent_split(node* root,
		      node*n,
		      int crash){

    node* new_root;

    // My journal wasn't written to properly; Ignore everything
    // we wrote in there.
    // Otherwise there is nothing to make consistent    
    if(n->log->consistent == 1 || n->log->consistent == 0 || n->log->split == 0){
	n->log->split = 0;
	n->log->consistent = 0;
	mem_flush(n->log);
	n->log->key = -1;
	return root;
    }
    
    // Now if I am here then either the journal was written to
    // properly and there is a pending split
    //*******************DANGER***************
    if(crash == 9){
	//printf("%sWas supposed to split node but system crashed. Should be fine%s\n", KRED, KWHT); 
	return root;
    }    
    //*****************************************
    
    // Get parent:
    node* parent = n->parent;
    bool change_flag = false;
												
    if(parent == NULL){
        // There was only one leaf node in the whole tree
        // which was the root
        parent = make_node();
        change_flag = true;
    }

    //*******************DANGER***************
    if(crash == 10){
	//printf("%sSystem crashed midway through splitting of leaf node%s\n", KRED, KWHT);    
	return root;
    }
    //*****************************************
    
    node* left = make_leaf();
    node* right = make_leaf();
    
    // inside split we look up the journal
    split(n, &left, &right);
    
    // first value of right goes into parent
    int key = *(right->keys);


    //*******************DANGER******************************************************
    if(crash == 11){
	//printf("%sSplit leaf node complete but didn't update parent before system crashed%s\n", KRED, KWHT);	    
	return root;
    }    
    //********************************************************************************
    
    // PARENT has room 
    if(parent->num_keys < (order -1)){
	insert_into_parent_simple(parent,
				  key,
				  left,
				  right);
	
	if(change_flag){
	    if(crash == 12){
		printf("%sCompleted split and updated parent but crashed before we could update root%s\n", KRED, KWHT);	    
		return root;
	    }    
	    
	    // Now even if I crash before this then all is lost (root remains like it was)
	    root = parent;
	    // I crash now, my journal is updated so i can rebuild	    
	}
	return root;
    }    
    else{
        root = parent_split(root,
			    parent,
			    key,
			    left,
			    right,
			    crash);
	return root;
    }
    //============================================================
    // If it fails before this line then all work is lost
    // I haven't changed anything about the the status of my current
    // node n or the root of the tree
    
    // If I get here new_root is golden!!
    // I'm just going to free n : it might be slightly inefficient but
    
}

/*
 * Function: write_journal
 * --------------------
 *  Update journal for the node
 *
 *  root: root of the tree
 *  node: node to which I wish to add
 *  key: key to be inserted/ deleted
 *  left: asscociated value if leaf node, left ptr if internal node
 *  right: right ptr, if internal node, NULL if leaf
 *  split/borrow: 1 if this insert will lead to a split 0 otherwise/
            if this is a delete then 1 if delete will lead to borrowing 0 otherwise
 *  crash: int markers for where the program shoud
 *  del: 1 if this key is to be deleted 0 otherwise
 *
 *  returns: root : root node of updated tree
 *           
 */
int write_journal(node* root,
		  node* n,
		  int key,
		  void* left,
		  void*right,		  
		  int split,
		  int crash,
          int del){

    log_* l = n->log;


   if(l->del == 1){
    if(l->split != 0){
        make_consistent_delete_hard(n);
        l->split = 0;
        cflush(l);
        return 1;
    }
    int flag = check_consistency_delete_simple(n);
	make_consistent_delete_simple(n, flag);
    l->consistent = 0;
    cflush(l);
   }
    
    /* now check if it needs a split cleanup? */
    // The split can never be on without the key and value already written to the log
    // So i can always go ahead and re do the work    
    // The journal can never be in 1 state and have a pending split either
    if(n->log->split != 0){
    	/* node* root = find_root(n); */
	//printf("THIS NODE IS PENDING A SPLIT\n");
	//printf("Handle that business first before I let you write new stuff\n");
        //printf("THis key is sitting in my log %d\n", n->log->key);
	make_consistent_split(root, n, crash);
	mem_flush(n->log);
	n->log->split =0; // split is fine
	return 1;
    }
    // In case the journal is stale clean it up, then write the fresh one
    // just the node itself
    if(n->log->consistent == 2 ){
	int flag = check_consistency_simple(n);
	n = make_consistent_simple(n, flag);
	n->log->consistent = 0;
	cflush(n->log);        	
    }
    
    // Journal write begin
    l->consistent = 1;
    // If the journal wasn't consistent then node's keys woudn't get updated
    // so there's no chance of this getting corrupt :)
    
    //******DANGER***************
    if(crash == 6){
	//printf("%sJournal write incomplete I: Should ignore this write%s\n", KRED, KWHT);   
	return 0;
    }    
    //************************
    
    l->num_keys = n->num_keys + 1;
    cflush(l);
    //******DANGER***************
    if(crash == 7){
	//printf("%sJournal write incomplete II: Should ignore this write%s\n", KRED, KWHT);   
	return 0;
    }    
    //************************
    
    l->key = key;    
    if(n->is_leaf){
	l->left = (record*)left;
	cflush(l);
    }
    // it'll be a pointer to another node
    else{
	l->left = left;
	l->right = right;
    }
    //******DANGER***************
    if(crash == 8){
	//printf("%sJournal write incomplete III: Should ignore this write%s\n", KRED, KWHT);
	return 0;
    }        
    // Node has had value and key written to it, so now if I turn the split
    // flag I'll know how to split it up
    l->split = split;
    cflush(l); 
      
    l->consistent = 2;
    cflush(l);
    // Journal write end
    return 0;
}

/*
 * Function: insert
 * --------------------
 *  Main insert function to insert value in a B+ tree
 *
 *  root: root of the tree
 *  key: key to be inserted
 *  v: asscociated value
 *
 *  returns: root : root node of updated tree
 *           
 */
node* insert(node* root, int key, int v, int crash){
  
    // check if duplicate key if yea return out
    record* value = make_record(v);
    
    // check if root is null
    if(root == NULL){
        node* root = make_leaf();

	//******DANGER***************
	if(crash == 1){
	    printf("%sMade root node but didn't write to journal. Should ignore the write %s\n", KRED, KWHT);
	    return root;
	}
	//************************
	
        write_journal(root,
		      root,
		      key,
		      value,
		      NULL,
		      0, crash,0); // No split

	//******DANGER***************
	if(crash == 2){
	    printf("%sMade root node and wrote journal but didn't update tree. %s \n", KRED, KWHT);	    
	    return root;
	}
	//************************
	
        make_consistent_simple(root, 1);		
        // root's parent is already null(default value)
        return root;
    }

    // root isn't null
    node* leaf = find_leaf(root, key, false);
    
    // Space available
    if (leaf->num_keys < order - 1) {

	// I don't need to do any parent pointing business here: I'm
        // just adding to an array
        write_journal(root,
		      leaf,
		      key,
		      value,
		      NULL,
		      0, crash,0); // No split

	//******DANGER***************
	if(crash ==3){ // Leaf journal write but not made consistent
	    // printf("%sWrote leaf journal but program crashed before we could write to tree. Shouldn't matter%s \n", KRED, KWHT);	    
	    return root;
	}
	//************************
	
        make_consistent_simple(leaf, 1); // make it consistent with the journal
    }
    // SPLITTING TIME
    else{
	// Split leaf	
        write_journal(root,
		      leaf,
		      key,
		      value,
		      NULL,
		      1, crash,0);
	
        // leaf gets destroyed every time

	//******DANGER***************
	if(crash == 5){
	    // printf("%sWrote Journal of leaf node but didn't update tree.%s\n", KRED, KWHT);
	    return root;
	}
	//***************************

        root = make_consistent_split(root, leaf, crash);
    }
        
    return root;
}

unsigned int rand_interval(unsigned int min, unsigned int max)
{
    int r;
    const unsigned int range = 1 + max - min;
    const unsigned int buckets = RAND_MAX / range;
    const unsigned int limit = buckets * range;

    /* Create equal size buckets all in a row, then fire randomly towards
     * the buckets until you land in one of them. All buckets are equally
     * likely. If you land off the end of the line of buckets, try again. */
    do
	{
	    r = rand();
	} while (r >= limit);

    return min + (r / buckets);
}

node*
traverse(node*root, node* n){

    if(n->is_leaf){
	/* printf("Split status: Consistency %d %d\n", n->log->split, n->log->consistent); */
	if(n->log->split){	    
	    root = make_consistent_split(root,n, 0);
	    return root;
	}
	else{
	    n = make_consistent_simple(n,1);
	}	 
    }
    else{
	int i;
	for(i=0; i<n->num_keys+1; i++){
	    root = traverse(root, n->pointers[i]);	    
	}	
    }

    return root;	
}

int main(int argc, char* argv[]){
    node* root = NULL;
    int i,x=-1000;
    uint64_t diff;
    struct timespec start,end;
    
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
    for(i=0; i< atoi(argv[1]); i++){
	/* x = rand_interval(0,10); */
	x++;
        //printf("INSERTING %d Random number generated: %d\n", 2*i, x);	
        root = insert(root,2*i, 9, x);	
	//printf("\n");

	//printf("STATE IMMEDIATELY AFTER CRASH\n");
	//print_tree(root);
	if(x>0){
	    root = traverse(root, root);
	}
	//printf("\nSTATE AFTER JOURNAL PLAYBACK\n");	
	//print_tree(root);
	//printf("========================================\n");
    }
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID,&end);
    diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
    diff = diff/100000;
    printf("Time (sec): %llu\n",(long long unsigned int)diff);
   //root = insert(root,3,9);
    int d;
    //root = insert(root,7,9,x);
    //print_tree(root);
    //printf("DELETE\n");
    //d = delete_key(root,atoi(argv[2]),x);
    //print_tree(root);
    //printf("Order is: %d\n",order);
    
    /* node* leaf = find_leaf(root, 6, false); */
    /* print_node(leaf->parent); */
    /* leaf = find_leaf(root, 18, false); */
    /* print_node(leaf->parent); */
    /* leaf = find_leaf(root, 14, false); */
    /* print_node(leaf->parent); */
    /* leaf = find_leaf(root, 10, false); */
    /* print_node(leaf->parent); */
    
    return 0;
}
