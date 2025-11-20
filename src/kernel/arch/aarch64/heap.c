#include "heap.h"
#include "mm.h"
#include "printf.h"


static unsigned long heap_start;
static unsigned long heap_end;
static heap_block_t* first_block;

void heap_init() {
    // We need to request 256 consecutive pages
    // This needs to be called before scheduler pops off

    heap_start = allocate_kernel_page();
    printf("Heap start allocated at: 0x%x\n", heap_start);
    for (int i = 1; i < 256; i++) {
        // Nothing else requests pages so are next to each other
        heap_end = allocate_kernel_page(); 
    }
    printf("Heap end allocated at: 0x%x\n", heap_end);

    first_block = (heap_block_t *)heap_start;
    first_block->is_used = 0;
    first_block->next_block = 0;
    first_block->prev_block = 0;
}

/*
------------------------------------------
| heap_block_t | some mem | heap_block_t |
------------------------------------------
*/

void *kmalloc(unsigned long size) {
    // Check block by block until we find a free one or last
    heap_block_t *free_block_ptr = first_block;
    while (free_block_ptr != 0) {
        if (!free_block_ptr->is_used)
            break;
        else
            free_block_ptr = free_block_ptr->next_block;
    }
    // We should either get nothing or block
    if (free_block_ptr == 0)
        return 0; // We have nothing
    
    // We have something

    heap_block_t* old_next_ptr = free_block_ptr->next_block;

    heap_block_t* next_block_addr = (heap_block_t*)((char*) free_block_ptr + sizeof(heap_block_t) + size);

    if (next_block_addr > (heap_block_t*)heap_end || 
        ((free_block_ptr->next_block > 0) && (next_block_addr > free_block_ptr->next_block)))
        // Does it fit within heap, and does it fit within the block, given we have next block
        return 0; // We ain't got no heap

    free_block_ptr->is_used = 1;
    

    free_block_ptr->next_block = next_block_addr;
    // Set up rest of free space
    free_block_ptr->next_block->is_used = 0;
    free_block_ptr->next_block->next_block = old_next_ptr; // Inserted free remainder points further
    free_block_ptr->next_block->prev_block = free_block_ptr; // And points back to parent

    return (char*)free_block_ptr + sizeof(heap_block_t);
}

void kfree(void *ptr) {
    //TODO: check if this works fully, I don't use it much yet
    // See where it points to
    heap_block_t* block = (heap_block_t*)((char*)ptr - sizeof(heap_block_t));

    if (!block->is_used)
        return; // Nothing to free

    // If prev is also free, merge with ours
    if (block->prev_block != 0 && !block->prev_block->is_used) {
        heap_block_t* prev = block->prev_block;
        prev->next_block = block->next_block;
        block = prev;
    }
    // If next is free, merge with ours
    if (block->next_block != 0 && !block->next_block->is_used) {
        heap_block_t* next = block->next_block;
        block->next_block = next->next_block;
    }
    // Ensure it is free and we're done
    block->is_used = 0;
}
