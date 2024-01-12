/*
All the main functions with respect to the MeMS are inplemented here
read the function discription for more details

NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
you are only allowed to implement the functions 
you can also make additional helper functions a you wish

REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
*/
// add other headers as required
#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>


#define PAGE_SIZE 4096

#define PROCESS 1 
#define HOLE 0

// All the main functions with respect to the MeMS are inplemented here
// read the function discription for more details

// NOTE: DO NOT CHANGE THE NAME OR SIGNATURE OF FUNCTIONS ALREADY PROVIDED
// you are only allowed to implement the functions 
// you can also make additional helper functions a you wish

// REFER DOCUMENTATION FOR MORE DETAILS ON FUNSTIONS AND THEIR FUNCTIONALITY
// */
// add other headers as required
/*
Use this macro where ever you need PAGE_SIZE.
As PAGESIZE can differ system to system we should have flexibility to modify this 
macro to make the output of all system same and conduct a fair evaluation. 
*/
// #define head 
// #define PAGE_SIZE 4096
// #define PAGE_SIZE 4096

struct sub_chain_node
{
    size_t size ;
    int type ;
    struct sub_chain_node* next ;
    struct sub_chain_node* prev ;
    void* memAddress;
    void* vAddress;

};

struct main_chain_node 
{
    size_t size ;
    struct sub_chain_node* sub_chain_head;
    struct MainChainNode* down;
    struct MainChainNode* up;
    void* memAddress;
    void* vAddress;


};


static struct sub_chain_node* create_sub_chain_node(size_t size, int type)
{
    size_t total_size = sizeof(struct sub_chain_node);
    struct sub_chain_node* new_node  = (struct sub_chain_node*)mmap(NULL,total_size,PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
};

int countMainChainNodes(struct main_chain_node* head ){
    int count=0;
    while(head!=NULL){
        count++;
        head=head->down;
    }
    return count;
}


int countSideChainNodes(struct sub_chain_node* head ){
    int count=0;
    while(head!=NULL){
        count++;
        head=head->next;
    }
    return count;
}

struct main_chain_node* addMainChainNode(struct main_chain_node* head, size_t size, void* memAddress, void* vAddress) {
    if (countMainChainNodes(head) == 0) {
        struct main_chain_node* temp = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        temp->sub_chain_head = NULL;
        temp->down = NULL;
        return temp;
    }

    struct main_chain_node* iterateHead = head;
    while (iterateHead->down != NULL) {
        iterateHead = iterateHead->down;
    }

    struct main_chain_node* temp = mmap(NULL, size, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
    iterateHead -> down = temp;
    temp -> sub_chain_head = NULL;
    temp -> down = NULL;
    return head;
}



void addSideChainNode(struct main_chain_node* head, struct sub_chain_node* subNode, size_t size, int type, void* memAddress) {
    if (head->sub_chain_head == NULL) {
        struct sub_chain_node* temp = mmap(NULL, sizeof(struct sub_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        temp->type = type;
        temp->size = size;
        temp->next = NULL;
        temp->prev = NULL;
        temp -> memAddress = memAddress;
        head->sub_chain_head = temp;
    } else {
        struct sub_chain_node* temp = mmap(NULL, sizeof(struct sub_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        temp -> next = subNode -> next;
        if(temp -> next != NULL){
            temp -> next -> prev = temp;
        }
        subNode -> next = temp;
        temp->type = type;
        temp->size = size;
        temp->next = NULL;
        temp->prev = subNode;
        temp -> memAddress = memAddress;
        return;
    }
}

// void deleteSideChainNode(struct main_chain_node* mainNode, struct sub_chain_node* subNode){
//     //if theres only one node
//     if((subNode -> next == NULL) && (subNode -> prev == NULL)){
//         munmap(subNode, sizeof(struct sub_chain_node));
//         mainNode -> sub_chain_head = NULL;
//         return;
//     }
// }
void deleteSideChainNode(struct main_chain_node* mainNode, struct sub_chain_node* subNode) {
    if (subNode == NULL) {
        // Invalid subNode, handle error or return.
        return;
    }

    if (subNode->prev == NULL && subNode->next == NULL) {
        // If there's only one node
        munmap(subNode, sizeof(struct sub_chain_node));
        mainNode->sub_chain_head = NULL;
        return;
    }

    if (subNode->prev == NULL) {
        // If subNode is the head of the sub-chain
        mainNode->sub_chain_head = subNode->next;
        subNode->next->prev = NULL;
        munmap(subNode, sizeof(struct sub_chain_node));
        return;
    }

    if (subNode->next == NULL) {
        // If subNode is the tail of the sub-chain
        subNode->prev->next = NULL;
        munmap(subNode, sizeof(struct sub_chain_node));
        return;
    }

    // Sub-node is somewhere in the middle
    subNode->prev->next = subNode->next;
    subNode->next->prev = subNode->prev;
    munmap(subNode, sizeof(struct sub_chain_node));
}


/*
Initializes all the required parameters for the MeMS system. The main parameters to be initialized are:
1. the head of the free list i.e. the pointer that points to the head of the free list
2. the starting MeMS virtual address from which the heap in our MeMS virtual address space will start.
3. any other global variable that you want for the MeMS implementation can be initialized here.
Input Parameter: Nothing
Returns: Nothing
*/
struct main_chain_node* main_chain_head;

// head ko mmap() ka physical address dena hai k
void mems_init()
{
    main_chain_head = NULL;
}


/*
This function will be called at the end of the MeMS system and its main job is to unmap the 
allocated memory using the munmap system call.
Input Parameter: Nothing
Returns: Nothing
*/
void mems_finish()
{
    
}


/*
Allocates memory of the specified size by reusing a segment from the free list if 
a sufficiently large segment is available. 

Else, uses the mmap system call to allocate more memory on the heap and updates 
the free list accordingly.

Note that while mapping using mmap do not forget to reuse the unused space from mapping
by adding it to the free list.
Parameter: The size of the memory the user program wants
Returns: MeMS Virtual address (that is created by MeMS)
*/ 
void* mems_malloc(size_t size) {
    // Calculate rounded_size
    size_t rounded_size = ((size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

    // Iterate through the main nodes
    struct main_chain_node* main_node = main_chain_head;
    while (main_node != NULL) {
        // Iterate through the sub chain
        struct sub_chain_node* sub_node = main_node->sub_chain_head;
        while (sub_node != NULL) {
            if (sub_node->type == HOLE && sub_node->size >= size) {
                // Found a suitable hole. Allocate a new sub chain node in front of the hole.

                // If size is strictly greater
                if (sub_node->size > size) {
                    // Allocate a new sub_chain_node
                    struct sub_chain_node* new_node = (struct sub_chain_node*)mmap(NULL, sizeof(struct sub_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

                    new_node->next = sub_node;
                    if (sub_node->prev != NULL) {
                        new_node->prev = sub_node->prev;
                        sub_node->prev->next = new_node;
                        sub_node->prev = new_node;
                    } else {
                        sub_node->prev = new_node;
                        main_node->sub_chain_head = new_node;
                    }

                    // Update sub nodes' attributes and size
                    sub_node->type = PROCESS;

                    // Fill new nodes' attributes
                    new_node->type = HOLE;
                    new_node->size = sub_node->size - size;
                    sub_node->size = size;
                    new_node->memAddress = sub_node->memAddress + size;
                    new_node->vAddress = sub_node->vAddress + size;

                    return sub_node->vAddress;
                }
                // If size is equal
                else if (sub_node->size == size) {
                    sub_node->type = PROCESS;
                }
            }
            sub_node = sub_node->next;
        }
        main_node = main_node->down;
    }

    // If no suitable hole was found, allocate a new main chain node
    void* memAddress = mmap(NULL, rounded_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    // Create a new main_chain_node
    struct main_chain_node* new_main_node = mmap(NULL, sizeof(struct main_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);

    if (main_chain_head == NULL) {
        // If the main_chain is empty, initialize it
        main_chain_head = new_main_node;
        main_chain_head->down = NULL;
        main_chain_head->up = NULL;
        main_chain_head->sub_chain_head = NULL;
        main_chain_head->size = rounded_size;
        main_chain_head->memAddress = memAddress;
        main_chain_head->vAddress = (void*)0;

        // Create a new sub chain node
        struct sub_chain_node* new_sub_node = mmap(NULL, sizeof(struct sub_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        new_sub_node->type = PROCESS;
        new_sub_node->size = size;
        new_sub_node->next = NULL;
        new_sub_node->prev = NULL;
        new_sub_node->memAddress = memAddress;
        new_sub_node->vAddress = main_chain_head->vAddress;
        main_chain_head->sub_chain_head = new_sub_node;

        // Create a hole node if required
        if (rounded_size != size) {
            struct sub_chain_node* new_hole_node = mmap(NULL, sizeof(struct sub_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
            new_hole_node->type = HOLE;
            new_hole_node->size = rounded_size - size;
            new_hole_node->next = NULL;
            new_hole_node->prev = NULL;
            new_hole_node->memAddress = memAddress + size;
            new_hole_node->vAddress = new_sub_node->vAddress + size;
            new_sub_node->next = new_hole_node;
            new_hole_node->prev = new_sub_node;
        }

        return main_chain_head->vAddress;
    } else {
        // If the main_chain is not empty, add a new main chain node
        struct main_chain_node* iterateHead = main_chain_head;
        while (iterateHead->down != NULL) {
            iterateHead = iterateHead->down;
        }

        iterateHead->down = new_main_node;
        new_main_node->down = NULL;
        new_main_node->up = iterateHead;
        new_main_node->sub_chain_head = NULL;
        new_main_node->size = rounded_size;
        new_main_node->memAddress = memAddress;
        new_main_node->vAddress = iterateHead->vAddress + iterateHead->size;

        // Create a new sub chain node
        struct sub_chain_node* new_sub_node = mmap(NULL, sizeof(struct sub_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
        new_sub_node->type = PROCESS;
        new_sub_node->size = size;
        new_sub_node->next = NULL;
        new_sub_node->prev = NULL;
        new_sub_node->memAddress = memAddress;
        new_sub_node->vAddress = new_main_node->vAddress;
        new_main_node->sub_chain_head = new_sub_node;

        // Create a hole node if required
        if (rounded_size != size) {
            struct sub_chain_node* new_hole_node = mmap(NULL, sizeof(struct sub_chain_node), PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
            new_hole_node->type = HOLE;
            new_hole_node->size = rounded_size - size;
            new_hole_node->next = NULL;
            new_hole_node->prev = NULL;
            new_hole_node->memAddress = memAddress + size;
            new_hole_node->vAddress = new_sub_node->vAddress + size;
            new_sub_node->next = new_hole_node;
            new_hole_node->prev = new_sub_node;
        }

        return new_main_node->vAddress;
    }
}




/*
this function print the stats of the MeMS system like
1. How many pages are utilised by using the mems_malloc
2. how much memory is unused i.e. the memory that is in freelist and is not used.
3. It also prints details about each node in the main chain and each segment (PROCESS or HOLE) in the sub-chain.
Parameter: Nothing
Returns: Nothing but should print the necessary information on STDOUT
*/

void mems_print_stats()
{
    size_t total_pages = 0;
    size_t unused_memory = 0;
    size_t main_chain_length = 0;
    size_t sub_chain_lengths[100] = {0}; // Assuming there are at most 100 main chain nodes

    printf("\n--------- Printing Stats [mems_print_stats] --------\n");
    printf("mems SYSTEM STATS\n");

    struct main_chain_node* main_node = main_chain_head;
    while (main_node != NULL) {
        total_pages += main_node->size / PAGE_SIZE;
        printf("Main[%ld:%ld] <-> ", main_node->vAddress, (main_node->vAddress) + (main_node -> size) - 1);

        size_t sub_chain_length = 0;
        struct sub_chain_node* sub_node = main_node->sub_chain_head;
        while (sub_node != NULL) {
            if (sub_node->type == HOLE) {
                unused_memory += sub_node->size;
                printf("H[%ld:%ld] <-> ", sub_node->vAddress, (sub_node->vAddress) + (sub_node -> size) - 1);
            }else{
                printf("P[%ld:%ld] <-> ", sub_node->vAddress, (sub_node->vAddress) + (sub_node -> size) - 1);
            }
            sub_node = sub_node->next;
            sub_chain_length++;
        }
        sub_chain_lengths[main_chain_length] = sub_chain_length;
        main_chain_length++;
        main_node = main_node->down;
        printf("\n");
    }

    printf("NULL\n");
    printf("Pages used: %zu\n", total_pages);
    printf("Space unused: %zu\n", unused_memory);
    printf("Main Chain Length: %zu\n", main_chain_length);
    printf("Sub-chain Length array: [");
    for (size_t i = 0; i < main_chain_length; i++) {
        printf("%zu, ", sub_chain_lengths[i]);
    }
    printf("]\n");
}

/*
Returns the MeMS physical address mapped to ptr ( ptr is MeMS virtual address).
Parameter: MeMS Virtual address (that is created by MeMS)
Returns: MeMS physical address mapped to the passed ptr (MeMS virtual address).
*/

//     // iterate through the main nodes
//     // if virtual pointer is greater than equal to main node ka virtual address and less than main node ka virtual address + size
//     // similarly iterate through the sub chain
//     // return the physical address, given - starting address (of the node were at) and add that to the physical address.

//     // there are some edge cases: is v_ptr is already hole then give error
//     //                      if the v_ptr is given -ve or something

void* mems_get(void* v_ptr) {
    // // Get the MeMS physical address corresponding to a given MeMS virtual address
    struct main_chain_node* node = main_chain_head;
    while (node != NULL){
        if(v_ptr >= node->vAddress && v_ptr < (node->vAddress + node->size)) {
            struct sub_chain_node* subNode = node -> sub_chain_head;
            while(subNode != NULL){
                if(v_ptr >= subNode->vAddress && v_ptr < (subNode->vAddress + subNode->size)) {
                    return subNode->memAddress + (v_ptr - subNode->vAddress);
                }
                subNode = subNode->next;
            }
        }
        node = node->down;
    }
    // If we reach here, the virtual address was not found in any node
    // Handle this case as an error
    fprintf(stderr, "Error: Invalid virtual address\n");
    return NULL;
}

/*
this function free up the memory pointed by our virtual_address and add it to the free list
Parameter: MeMS Virtual address (that is created by MeMS) 
Returns: nothing
*/

    //we will be getting the virtual pointer

    // iterate through the main nodes
    //if virtual pointer is greater than equal to main node ka virtual address and less than main node ka virtual address + size
    // similarly iterate through the sub chain
    // once we find the node change it's type to HOLE
    // once we make it a HOLE check for contigous HOLES and merge them

    //there are some edge cases: is v_ptr is already hole then give error
    //                      if the v_ptr is given -ve or something

void mems_free(void *v_ptr) {
        // // Get the MeMS physical address corresponding to a given MeMS virtual address
    struct main_chain_node* node = main_chain_head;
    while (node != NULL){
        if(v_ptr >= node->vAddress && v_ptr < (node->vAddress + node->size)) {
            struct sub_chain_node* subNode = node -> sub_chain_head;
            while(subNode != NULL){
                if(v_ptr >= subNode->vAddress && v_ptr < (subNode->vAddress + subNode->size)) {
                    return subNode->type;
                }
                subNode = subNode->next;
            }
        }
        node = node->down;
    }
    // If we reach here, the virtual address was not found in any node
    // Handle this case as an error
    fprintf(stderr, "Error: Invalid virtual address\n");
    return NULL;
}


