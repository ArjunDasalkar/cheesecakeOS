#ifndef CK_PMEM_H
#define CK_PMEM_H

#include <stdint.h>

/*
 * Physical memory allocator.
 * Manages 4KB pages of physical RAM.
 */

/* Page size (4KB) */
#define CK_PAGE_SIZE 4096

/* Page frame number (physical_address / 4096) */
typedef uint32_t ck_pfn_t;

/*
 * Initialize physical memory allocator.
 * Must be called after kernel boots with GRUB Multiboot2 info.
 * Parses memory map to determine which pages are available.
 */
void ck_pmem_init(void *multiboot_info);

/*
 * Allocate a single 4KB page.
 * Returns the physical address (or 0 if no memory available).
 */
uint32_t ck_pmem_alloc_page(void);

/*
 * Free a previously allocated page.
 * addr should be page-aligned (multiple of 4096).
 */
void ck_pmem_free_page(uint32_t addr);

/*
 * Get total available physical memory (bytes).
 */
uint32_t ck_pmem_get_total_memory(void);

/*
 * Get number of free pages.
 */
uint32_t ck_pmem_get_free_pages(void);

/*
 * Get number of used pages.
 */
uint32_t ck_pmem_get_used_pages(void);

#endif
