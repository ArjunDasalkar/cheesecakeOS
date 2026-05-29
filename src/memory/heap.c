#include "heap.h"
#include "pmem.h"

/*
 * Simple heap allocator using a freelist.
 * Each allocated block has a header with size and "allocated" flag.
 * Free blocks are linked in a freelist for reuse.
 */

struct ck_heap_header {
    uint32_t size;         /* Size of block (including header) */
    uint8_t allocated;     /* 1 if allocated, 0 if free */
    uint8_t padding[3];    /* Unused */
    struct ck_heap_header *next;  /* Next free block in freelist */
};

#define CK_HEAP_HEADER_SIZE sizeof(struct ck_heap_header)

/* Heap statistics */
static uint32_t ck_heap_total_allocated = 0;
static uint32_t ck_heap_total_freed = 0;

/* Head of freelist */
static struct ck_heap_header *ck_heap_freelist = NULL;

/* Current heap size (in bytes) */
static uint32_t ck_heap_size = 0;

/* Maximum heap size (256MB at 0xD0000000 - 0xDFFFFFFF) */
#define CK_HEAP_MAX_SIZE (256 * 1024 * 1024)

/*
 * Request more memory from the physical allocator to expand the heap.
 * Allocates one page (4KB) at a time.
 */
static void *ck_heap_expand(uint32_t needed_size) {
    if (ck_heap_size + needed_size > CK_HEAP_MAX_SIZE) {
        return NULL;  /* Heap exhausted */
    }
    
    /* Allocate physical page */
    uint32_t phys_addr = ck_pmem_alloc_page();
    if (phys_addr == 0) {
        return NULL;  /* No free pages */
    }
    
    /* For now, use direct physical addressing (identity-mapped) */
    /* In a full VM system, we'd map this to virtual heap space */
    uint8_t *heap_base = (uint8_t *)phys_addr;
    
    ck_heap_size += 4096;
    
    return heap_base;
}

/*
 * Initialize the heap.
 * Allocates the first page for the heap.
 */
void ck_heap_init(void) {
    ck_heap_freelist = NULL;
    ck_heap_size = 0;
    ck_heap_total_allocated = 0;
    ck_heap_total_freed = 0;
    
    /* Allocate first page */
    uint8_t *first_page = (uint8_t *)ck_heap_expand(4096);
    if (first_page == NULL) {
        return;  /* Failed to allocate first page */
    }
    
    /* Create a large free block covering the entire first page */
    struct ck_heap_header *header = (struct ck_heap_header *)first_page;
    header->size = 4096;
    header->allocated = 0;
    header->next = NULL;
    
    ck_heap_freelist = header;
}

/*
 * Allocate memory from the heap.
 * Uses first-fit strategy on the freelist.
 */
void *ck_malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    
    /* Add header size to allocation */
    uint32_t total_size = size + CK_HEAP_HEADER_SIZE;
    
    /* Search freelist for a suitable block */
    struct ck_heap_header *current = ck_heap_freelist;
    struct ck_heap_header *prev = NULL;
    (void)prev;  /* Unused in current simple implementation */
    
    while (current != NULL) {
        if (!current->allocated && current->size >= total_size) {
            /* Found a suitable block */
            struct ck_heap_header *result = current;
            
            /* If block is larger than needed, split it */
            if (current->size > total_size) {
                struct ck_heap_header *new_block;
                new_block = (struct ck_heap_header *)((uint8_t *)current + total_size);
                
                new_block->size = current->size - total_size;
                new_block->allocated = 0;
                new_block->next = current->next;
                
                current->size = total_size;
                current->next = new_block;
            }
            
            /* Mark as allocated */
            result->allocated = 1;
            ck_heap_total_allocated += total_size;
            
            /* Return pointer to data (after header) */
            return (void *)((uint8_t *)result + CK_HEAP_HEADER_SIZE);
        }
        
        prev = current;
        current = current->next;
    }
    
    /* No suitable block found, try to expand heap */
    uint8_t *new_heap = (uint8_t *)ck_heap_expand(total_size);
    if (new_heap == NULL) {
        return NULL;  /* Heap expansion failed */
    }
    
    /* Create header in new page */
    struct ck_heap_header *header = (struct ck_heap_header *)new_heap;
    header->size = total_size;
    header->allocated = 1;
    
    /* Link to freelist if there's remaining space */
    if (4096 > total_size) {
        struct ck_heap_header *remaining = (struct ck_heap_header *)(new_heap + total_size);
        remaining->size = 4096 - total_size;
        remaining->allocated = 0;
        remaining->next = ck_heap_freelist;
        header->next = remaining;
        ck_heap_freelist = remaining;
    } else {
        header->next = ck_heap_freelist;
        ck_heap_freelist = header;
    }
    
    ck_heap_total_allocated += total_size;
    
    /* Return pointer to data */
    return (void *)(new_heap + CK_HEAP_HEADER_SIZE);
}

/*
 * Free memory back to the heap.
 * Blocks are returned to the freelist for reuse.
 */
void ck_free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    
    /* Get header (located before the data) */
    struct ck_heap_header *header = (struct ck_heap_header *)((uint8_t *)ptr - CK_HEAP_HEADER_SIZE);
    
    if (!header->allocated) {
        return;  /* Double-free error, ignore */
    }
    
    /* Mark as free */
    header->allocated = 0;
    ck_heap_total_freed += header->size;
    
    /* Insert into freelist (simple: just add to front) */
    /* In a real implementation, we'd coalesce adjacent free blocks */
    header->next = ck_heap_freelist;
    ck_heap_freelist = header;
}

/*
 * Get heap statistics.
 */
uint32_t ck_heap_get_total_allocated(void) {
    return ck_heap_total_allocated;
}

uint32_t ck_heap_get_total_freed(void) {
    return ck_heap_total_freed;
}
