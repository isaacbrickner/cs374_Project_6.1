#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

#define ALIGNMENT 16

#define GET_PAD(x) ((ALIGNMENT - 1) - (((x) - 1) & (ALIGNMENT - 1)))
#define PADDED_SIZE(x) ((x) + GET_PAD(x))
#define PTR_OFFSET(p, offset) ((void*)((char *)(p) + (offset)))

struct block {
    int size;            // Size in bytes
    int in_use;          // Boolean
    struct block *next;
};

// initialize null head
struct block *head = NULL;


void *myalloc(int byte_size) {
    // allocating the space
    if (head == NULL) {
        // one time retrieval of memory from the OS 
        void *heap = mmap(NULL, 1024, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        
        // initialize the head block of contiguous memory
        head = (struct block *)heap;
        head->size = 1024 - PADDED_SIZE(sizeof(struct block)); // pad the size of the block from 12 to 16 bytes to keep alignment in check 
        head->in_use = 0;
        head->next = NULL;
    }

    // splitting the blocks and indicate remaining memory
    struct block *current = head;

    // check if a block exists then see whether its being used. also checks if the block allocate memory size is correctly aligned
    while (current != NULL) { 
        if (!current->in_use && current->size >= PADDED_SIZE(byte_size)) {

            // check if the block is big enough to split
            int space_needed = PADDED_SIZE(byte_size) + PADDED_SIZE(sizeof(struct block)) + ALIGNMENT; // calc the size of needed space accounting for padding for the block and request space

            if (current->size >= space_needed) {

                // split the block and reattach the blocks
                struct block *new_block = (struct block *)PTR_OFFSET(current, PADDED_SIZE(byte_size)); // new block
                new_block->size = current->size - PADDED_SIZE(byte_size) - PADDED_SIZE(sizeof(struct block)); // assign remaining size of alloc'd memory to the new block
                new_block->in_use = 0;
                new_block->next = current->next;
                
                current->size = PADDED_SIZE(byte_size);
                current->in_use = 1;
                current->next = new_block;
                
                return PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)));
            } else { // if block is too small, mark as used 
                current->in_use = 1;
                return PTR_OFFSET(current, PADDED_SIZE(sizeof(struct block)));
            }
        }
        current = current->next;
    }

    //no block
    return NULL;
}

// added coalescing of memory blocks
void myfree(void *ptr) {
    struct block *current = head;
    struct block *prev = NULL;

    while (current != NULL) {
        if ((void *)current + PADDED_SIZE(sizeof(struct block)) == ptr) { 
            current->in_use = 0;

            if (current->next != NULL && !(current->next->in_use)) {
                current->size += PADDED_SIZE(sizeof(struct block)) + current->next->size;
                current->next = current->next->next;
            }

            if (prev != NULL && !(prev->in_use)) {
                prev->size += PADDED_SIZE(sizeof(struct block)) + current->size;
                prev->next = current->next;
            }

            return;
        }
        prev = current;
        current = current->next;
    }
}

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
    // copied example 4 from the canvas page
    void *p, *q, *r, *s;

    p = myalloc(10); print_data();
    q = myalloc(20); print_data();
    r = myalloc(30); print_data();
    s = myalloc(40); print_data();

    myfree(q); print_data();
    myfree(p); print_data();
    myfree(s); print_data();
    myfree(r); print_data();
        
    return 0;
}
