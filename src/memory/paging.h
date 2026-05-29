#ifndef CK_PAGING_H
#define CK_PAGING_H

#include <stdint.h>

/*
 * Paging setup and virtual memory management.
 */

/*
 * Initialize paging.
 * Creates page directory and page tables, enables paging in CR0/CR3.
 * Initially identity-maps the kernel and first 256MB of RAM.
 */
void ck_paging_init(void);

/*
 * Map a virtual page to a physical page.
 * Both vaddr and paddr should be page-aligned (multiple of 4096).
 * 
 * Returns 1 on success, 0 on failure.
 */
uint32_t ck_paging_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags);

/*
 * Unmap a virtual page (marks page table entry as not present).
 */
void ck_paging_unmap_page(uint32_t vaddr);

/*
 * Get the physical address mapped to a virtual address.
 * Returns the physical address, or 0 if not mapped.
 */
uint32_t ck_paging_virt_to_phys(uint32_t vaddr);

/* Paging flags for page table entries */
#define CK_PAGE_PRESENT   0x001
#define CK_PAGE_WRITE     0x002
#define CK_PAGE_USER      0x004
#define CK_PAGE_KERNEL    (CK_PAGE_PRESENT | CK_PAGE_WRITE)

#endif
