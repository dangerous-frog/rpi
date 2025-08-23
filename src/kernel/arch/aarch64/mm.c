#include "mm.h"

static unsigned short mem_map [PAGING_PAGES] = {0, };

unsigned long get_free_page() {
    for (int i = 0; i < PAGING_PAGES; i++) {
        if (mem_map[i] == 0) {
            mem_map[i] = 1;
            return LOW_MEMORY + i * PAGE_SIZE;
        }
    }
    return 0;
}

// void memzero(unsigned long src, unsigned long n) {
//     asm volatile (
//         "1: str xzr, [%0], #8\n"
//         "   subs %1, %1, #8\n"
//         "   b.gt 1b\n"
//         : "+r" (src), "+r" (n)
//         :
//         : "memory"
//     );
// }

// void memcpy(unsigned long dst, unsigned long src, unsigned long n) {
//     asm volatile (
//         "1: ldr x3, [%1], #8\n"
//         "   str x3, [%0], #8\n"
//         "   subs %2, %2, #8\n"
//         "   b.gt 1b\n"
//         : "+r" (dst), "+r" (src), "+r" (n)
//         :
//         : "x3", "memory"
//     );
// }

void free_page(unsigned long p) {
    // This nicely tells us which page to release
    mem_map[(p - LOW_MEMORY) / PAGE_SIZE] = 0;
}