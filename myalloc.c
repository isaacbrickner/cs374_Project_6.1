#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define ALIGNMENT 16

#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

// block containing details of allocated space
struct block {
    int size;            // Size in bytes
    int in_use;          // Boolean
    struct block *next;
};

// initialize null head
struct block *head = NULL;


void *myalloc(int size) {
    if (head == NULL) {
        // one time retrieval of memory from the OS 
        void *heap = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        
        // initialize the head block of contiguous memory
        head = (struct block *)heap;
        head->size = 1024 - PADDED_SIZE(sizeof(struct block)); // pad the size of the block from 12 to 16 bytes to keep alignment in check 
        head->in_use = 0;
        head->next = NULL;
    }

    // Find a free block that's big enough
    struct block *current = head;
    while (current != NULL) { // check if a block exists then see whether it is in use or not. also checks if the blocks allocate memory size is correctly aligned
        if (!current->in_use && current->size >= PADDED_SIZE(size)) {

            current->in_use = 1; // set block to in use 
            
            return PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block))); // return the pointer offset to next block 
        }
        current = current->next;
    }

    // No suitable block found
    return NULL;
}

// Function to print the linked list
void print_data(void) {
    struct block *b = head;

    if (b == NULL) {
        printf("[empty]\n");
        return;
    }

    while (b != NULL) {
        printf("[%d,%s]", b->size, b->in_use ? "used" : "free");
        if (b->next != NULL) {
            printf(" -> ");
        }

        b = b->next;
    }

    printf("\n");
}

int main() {
    void *p;
    void *p2;

    // Test allocations
    print_data();
    p = myalloc(64);
    print_data();
    p2 = myalloc(16);
    print_data();
    // p = myalloc(16);
    printf("%p\n", p);
    
    return 0;
}
