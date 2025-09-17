#pragma once
#include "stdint.h"

/*
    Here is a simple heap allocator
    Supports only allocating and freeing but good enough for my OS
*/

#define HEAP_SIZE 256 * PAGE_SIZE // 1 MiB = 256 * 4KiB pages


typedef struct heap_block {
    uint8_t is_used; 
    struct heap_block* next_block; 
    struct heap_block* prev_block;
} heap_block_t;  


void heap_init();
void* kmalloc(unsigned long size);
void kfree(void* ptr);