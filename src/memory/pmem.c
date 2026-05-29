#include "pmem.h"

/*
 * Physical memory allocator using a bitmap.
 * Each bit in the bitmap represents one 4KB page.
 * Bit 0 = free, Bit 1 = allocated.
 */

/* Support up to 256MB of RAM (65536 pages) */
#define CK_MAX_PAGES 65536
#define CK_BITMAP_SIZE (CK_MAX_PAGES / 8)  /* One byte = 8 pages */

static uint8_t ck_pmem_bitmap[CK_BITMAP_SIZE];

static uint32_t ck_pmem_total_pages = 0;
static uint32_t ck_pmem_used_pages = 0;

/*
 * Multiboot2 structures for memory map parsing.
 */
struct ck_multiboot2_mmap_entry {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t reserved;
} __attribute__((packed));

/*
 * Set a bit in the bitmap (mark page as allocated).
 */
static void ck_pmem_set_bit(uint32_t page) {
    if (page >= CK_MAX_PAGES) return;
    uint32_t byte_idx = page / 8;
    uint32_t bit_idx = page % 8;
    ck_pmem_bitmap[byte_idx] |= (1 << bit_idx);
}

/*
 * Clear a bit in the bitmap (mark page as free).
 */
static void ck_pmem_clear_bit(uint32_t page) {
    if (page >= CK_MAX_PAGES) return;
    uint32_t byte_idx = page / 8;
    uint32_t bit_idx = page % 8;
    ck_pmem_bitmap[byte_idx] &= ~(1 << bit_idx);
}

/*
 * Test a bit in the bitmap.
 * Returns 1 if allocated, 0 if free.
 */
static uint32_t ck_pmem_test_bit(uint32_t page) {
    if (page >= CK_MAX_PAGES) return 1;  /* Out of range = allocated */
    uint32_t byte_idx = page / 8;
    uint32_t bit_idx = page % 8;
    return (ck_pmem_bitmap[byte_idx] >> bit_idx) & 1;
}

/*
 * Initialize physical memory allocator.
 * Parses Multiboot2 memory map and marks pages accordingly.
 * 
 * Note: For simplicity, we assume GRUB provides a valid memory map.
 * The multiboot_info pointer is from GRUB's Multiboot2 info structure.
 */
void ck_pmem_init(void *multiboot_info) {
    (void)multiboot_info;  /* Unused for now */
    /* Initialize bitmap as all free */
    for (uint32_t i = 0; i < CK_BITMAP_SIZE; i++) {
        ck_pmem_bitmap[i] = 0;
    }
    
    ck_pmem_total_pages = 0;
    ck_pmem_used_pages = 0;
    
    /*
     * For now, we'll use a simple heuristic:
     * Assume GRUB provides memory and mark pages appropriately.
     * 
     * In a real kernel, we'd parse the Multiboot2 info structure here.
     * For this exercise, we'll assume reasonable defaults:
     * - Kernel occupies pages 0x100000-0x200000 (1MB kernel)
     * - Mark everything below 1MB as reserved (BIOS, bootloader, etc.)
     * - Everything else is available
     */
    
    /* Mark pages 0-384 as used (0x0 - 0x180000 = first 1.5MB reserved for BIOS/firmware) */
    for (uint32_t page = 0; page < 384; page++) {
        ck_pmem_set_bit(page);
        ck_pmem_total_pages++;
        ck_pmem_used_pages++;
    }
    
    /* Assume kernel occupies pages 256-512 (0x100000 - 0x200000 = 1MB for kernel) */
    /* Already marked above, so no extra marking needed */
    
    /* Mark remaining pages as free (up to 256MB) */
    for (uint32_t page = 384; page < CK_MAX_PAGES; page++) {
        ck_pmem_clear_bit(page);
        ck_pmem_total_pages++;
    }
}

/*
 * Allocate a single 4KB page.
 * Returns physical address (0 if allocation failed).
 */
uint32_t ck_pmem_alloc_page(void) {
    /* Find first free page */
    for (uint32_t page = 0; page < CK_MAX_PAGES; page++) {
        if (!ck_pmem_test_bit(page)) {
            /* Found a free page */
            ck_pmem_set_bit(page);
            ck_pmem_used_pages++;
            return page * CK_PAGE_SIZE;
        }
    }
    
    /* No free pages */
    return 0;
}

/*
 * Free a previously allocated page.
 */
void ck_pmem_free_page(uint32_t addr) {
    /* Ensure address is page-aligned */
    if (addr % CK_PAGE_SIZE != 0) {
        return;  /* Invalid address */
    }
    
    uint32_t page = addr / CK_PAGE_SIZE;
    
    if (ck_pmem_test_bit(page)) {
        ck_pmem_clear_bit(page);
        ck_pmem_used_pages--;
    }
}

/*
 * Get total available physical memory (bytes).
 */
uint32_t ck_pmem_get_total_memory(void) {
    return ck_pmem_total_pages * CK_PAGE_SIZE;
}

/*
 * Get number of free pages.
 */
uint32_t ck_pmem_get_free_pages(void) {
    return ck_pmem_total_pages - ck_pmem_used_pages;
}

/*
 * Get number of used pages.
 */
uint32_t ck_pmem_get_used_pages(void) {
    return ck_pmem_used_pages;
}
