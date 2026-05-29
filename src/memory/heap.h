#ifndef CK_HEAP_H
#define CK_HEAP_H

#include <stdint.h>
#include <stddef.h>

/*
 * Heap allocator (malloc/free).
 * Simple allocator using a free-list strategy.
 */

/*
 * Initialize the heap.
 * Allocates initial heap space.
 */
void ck_heap_init(void);

/*
 * Allocate memory.
 * Returns pointer to allocated block, or NULL on failure.
 */
void *ck_malloc(size_t size);

/*
 * Free memory.
 * Coalesces adjacent free blocks when possible.
 */
void ck_free(void *ptr);

/*
 * Get heap statistics.
 */
uint32_t ck_heap_get_total_allocated(void);
uint32_t ck_heap_get_total_freed(void);

#endif
