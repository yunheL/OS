#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define KNRM  "\x1B[0m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

typedef struct record {
    int value;
} record;

// add it to the node structure 
typedef struct log_{
    int split; // 0 no split; 1: leaf split 2: parent split
    int num_keys;
    int key;
    void* left;
    void* right;
    int del;
    int consistent; // 0:yes 1: in progress 2: journal write complete
} log_;

typedef struct node {
    log_* log;
    void ** pointers;
    int * keys;
    struct node * parent;
    bool is_leaf;
    int num_keys;
    int space;
} node;

int order= 15;
int verbose_output = 0;

void get_right(node* n);
void get_left(node* n, int lkey);

// FUNCTIONS============================================
void make_spacing(node* root, int x){

    if(root == NULL)
	return;


    root->space = x;        
    if(root->is_leaf){
	return;
    }
    
    int i=0;
    x*=2;
    while(i < root->num_keys +1){
	make_spacing((node*)root->pointers[i], x);
	i++;
    }
}

void print_arr(int* arr, int num_keys, int space){
    
    int i;
    for(i=0; i<space*2-2; i++)
        fprintf(stderr, " ");
    
    for(i=0; i< num_keys; i++){
        fprintf(stderr, "%d ", *(arr+i));
    }
    fprintf(stderr, "\n");
}


node**
get_children(node* n){

    if(n->is_leaf)
        return NULL;

    
    node** children;
    int i=0;    
    return children;
}

node* q[1000];
int front=0, end=0, counter=0;


void print_node(node * n){


    if(n==NULL){
        fprintf(stderr, "NULL\n");
        return;
    }
    fprintf(stderr, "\nKEYs: \n");    
    print_arr(n->keys, n->num_keys, 0);
    printf("NUMBER OF KEYS : %d\n", n->num_keys);
    printf("LEAF? : %d\n\n", n->is_leaf);
}


void
print_tree(node* root){

    make_spacing(root, 1);
    
    printf("TREE:\n");
    if (root == NULL) {
	printf("Empty tree.\n");
	return;
    }
    
    node* curr = root;
    node** children;
    node* c;

    
    q[end] = curr;
    end++;
    counter++;   
    while(counter != 0){
        // De-q
        curr = q[front];
        front++;
        counter --;
        print_arr(curr->keys, curr->num_keys, curr->space);
        
        int i =0;
        if(!curr->is_leaf){
	    while(i < curr->num_keys+1){
		c = (node*)curr->pointers[i];                        
                q[end] = (node*)c;                
                end++;
                counter++;
                i++;
            }
        }            
    }
}


record* make_record(int value) {
    record * new_record = (record *)malloc(sizeof(record));
    if (new_record == NULL) {
	perror("Record creation.");
	exit(EXIT_FAILURE);
    }
    else {
	new_record->value = value;
    }
    return new_record;
}

void
delete_node(node *n){

    int i;

    // I CAN't delete what the pointers point to as they are pointed to by
    // new leaves
    /* // I only have that many */
    /* for(i=0; i<n->num_keys+1; i++) */
    /*     free((record*)n->pointers[i]); */
    
    free(n->keys);    
    free(n->pointers);
    free(n->log);

    // Finally free the node
    free(n);   
}

/* Traces the path from the root to a leaf, searching
 * by key.  Displays information about the path
 * if the verbose flag is set.
 * Returns the leaf containing the given key.
 */
node * find_leaf( node * root, int key, bool verbose ) {
    
    int i = 0;
    node * c = root;
    
    if (c == NULL) {
	if (verbose)
	    printf("Empty tree.\n");
	return c;
    }

    // This makes sure the current node is not a leaf. So we haven't
    // actually seen the switcheroo yet described at the bottom yet:)
    while (!c->is_leaf) {
	if (verbose) {
	    printf("[");
	    for (i = 0; i < c->num_keys - 1; i++)
		printf("%d ", c->keys[i]);
	    printf("%d] ", c->keys[i]);
	}

	i = 0;
	while (i < c->num_keys) {
	    if (key >= c->keys[i]) i++;
	    else break;
	}

	if (verbose)
	    printf("%d ->\n", i);

	// it'll take 0 if it's less than all
	// it'll take the last guy if greater than all (clever)
	// found correct index in i
	// pointers[i] stores an address, which used to just a value(record) when it's a leaf
	// but since this is an internal node it gets changed into a node
	c = (node *)c->pointers[i];
    }

    if (verbose) {
	printf("Leaf [");
	for (i = 0; i < c->num_keys - 1; i++)
	    printf("%d ", c->keys[i]);
	printf("%d] ->\n", c->keys[i]);
    }

    return c;
}



node* make_node(){
    
    // Allocate space for node 
    node * new_node;
    new_node = malloc(sizeof(node));    
    if (new_node == NULL) {
        perror("Node creation.");
        exit(EXIT_FAILURE);
    }

    // Allocate space for keys
    new_node->keys = malloc( (order - 1) * sizeof(int) );
    if (new_node->keys == NULL) {
        perror("New node keys array.");
        exit(EXIT_FAILURE);
    }
    // Allocate space for references
    // pointers are pointers to pointers: so they are just addresses
    new_node->pointers = malloc( order * sizeof(void *));    
    if (new_node->pointers == NULL) {
        perror("New node pointers array.");
        exit(EXIT_FAILURE);
    }
    // INITIALLY ALL POINTERS POINT TO NOTHING
    int i;
    for(i=0; i<order; i++){
        *(new_node->pointers + i) = NULL;
    }
            
    new_node->log = malloc(sizeof(log_));
    if (new_node->pointers == NULL) {
        perror("No space for LOG.");
        exit(EXIT_FAILURE);
    }

    // Log isn't active
    new_node->log->consistent=1; //node just created nothing's been put into it, so it's incosistent
    new_node->log->split = 0;
    new_node->space=1;
    new_node->is_leaf = false;
    new_node->num_keys = 0;
    new_node->parent = NULL;
   
    return new_node;    
}


node* make_leaf(){
    node * leaf = make_node();
    leaf->is_leaf = true;
    // while making a new leaf the last guy always points to nothing
    leaf->pointers[order - 1]  = NULL;
    return leaf;
}


