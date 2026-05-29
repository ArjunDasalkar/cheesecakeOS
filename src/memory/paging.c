#include "paging.h"
#include "pmem.h"

/*
 * x86 32-bit paging structures.
 * 
 * Virtual address layout (32-bit):
 *   Bits 31-22: Page Directory Index (10 bits)
 *   Bits 21-12: Page Table Index (10 bits)
 *   Bits 11-0:  Page Offset (12 bits)
 * 
 * Page Directory Entry / Page Table Entry format:
 *   Bit 0: Present
 *   Bit 1: Read/Write (1 = writable)
 *   Bit 2: User/Supervisor (1 = user-accessible)
 *   Bits 11-3: Reserved (0)
 *   Bits 31-12: Physical address (of PT or page)
 */

/* Page directory and tables */
#define CK_PAGE_DIR_COUNT     1   /* 1 page directory = 1024 entries */
#define CK_PAGE_TABLE_COUNT   256 /* Up to 256 page tables (1GB virtual space) */

typedef uint32_t ck_pde_t;  /* Page Directory Entry */
typedef uint32_t ck_pte_t;  /* Page Table Entry */

/* Page directory: 1024 entries */
static ck_pde_t ck_page_directory[1024] __attribute__((aligned(4096)));

/* Page tables: dynamically allocated, but we'll reserve space for initial setup */
static ck_pte_t ck_page_tables[256][1024] __attribute__((aligned(4096)));

static uint32_t ck_page_table_count = 0;

/*
 * Extract page directory index from virtual address.
 */
static uint32_t ck_paging_get_pd_index(uint32_t vaddr) {
    return (vaddr >> 22) & 0x3FF;
}

/*
 * Extract page table index from virtual address.
 */
static uint32_t ck_paging_get_pt_index(uint32_t vaddr) {
    return (vaddr >> 12) & 0x3FF;
}

/*
 * Create a page directory entry.
 */
static ck_pde_t ck_paging_create_pde(uint32_t pt_address, uint32_t flags) {
    return (pt_address & 0xFFFFF000) | (flags & 0xFFF);
}

/*
 * Create a page table entry.
 */
static ck_pte_t ck_paging_create_pte(uint32_t page_address, uint32_t flags) {
    return (page_address & 0xFFFFF000) | (flags & 0xFFF);
}

/*
 * Flush TLB (Translation Lookaside Buffer) after changing a page table entry.
 * This tells the CPU to reload the page table from memory.
 */
static void ck_paging_flush_tlb(uint32_t vaddr) {
    __asm__ volatile ("invlpg (%0)" : : "r"(vaddr));
}

/*
 * Enable paging.
 * Sets CR3 to point to page directory, then sets PG bit in CR0.
 */
static void ck_paging_enable(void) {
    /* Load CR3 with page directory address */
    uint32_t pd_addr = (uint32_t)&ck_page_directory;
    __asm__ volatile (
        "mov %0, %%cr3"
        : : "r"(pd_addr)
    );
    
    /* Set PG bit (bit 31) in CR0 to enable paging */
    __asm__ volatile (
        "mov %%cr0, %%eax\n\t"
        "or $0x80000000, %%eax\n\t"
        "mov %%eax, %%cr0"
        : : : "eax"
    );
}

/*
 * Initialize paging.
 * Creates page directory and page tables for identity-mapping kernel.
 */
void ck_paging_init(void) {
    /* Clear page directory */
    for (uint32_t i = 0; i < 1024; i++) {
        ck_page_directory[i] = 0;
    }
    
    ck_page_table_count = 0;
    
    /*
     * Identity-map first 256MB of memory (256 page tables).
     * Virtual address = Physical address.
     * This allows the kernel to run without address translation.
     */
    for (uint32_t pt_idx = 0; pt_idx < 256; pt_idx++) {
        /* Clear this page table */
        for (uint32_t i = 0; i < 1024; i++) {
            ck_page_tables[pt_idx][i] = 0;
        }
        
        /* Create page table entries for this page table (256 * 1024 = 256K pages = 256MB) */
        uint32_t pt_base_addr = (uint32_t)&ck_page_tables[pt_idx];
        
        for (uint32_t pte_idx = 0; pte_idx < 1024; pte_idx++) {
            uint32_t phys_addr = (pt_idx * 1024 * 4096) + (pte_idx * 4096);
            ck_page_tables[pt_idx][pte_idx] = ck_paging_create_pte(
                phys_addr,
                CK_PAGE_KERNEL
            );
        }
        
        /* Create page directory entry pointing to this page table */
        ck_page_directory[pt_idx] = ck_paging_create_pde(
            pt_base_addr,
            CK_PAGE_KERNEL
        );
        
        ck_page_table_count++;
    }
    
    /* Enable paging */
    ck_paging_enable();
}

/*
 * Map a virtual page to a physical page.
 */
uint32_t ck_paging_map_page(uint32_t vaddr, uint32_t paddr, uint32_t flags) {
    /* Get indices into page directory and page table */
    uint32_t pd_idx = ck_paging_get_pd_index(vaddr);
    uint32_t pt_idx = ck_paging_get_pt_index(vaddr);
    
    /* Check if page directory entry is present */
    if (!(ck_page_directory[pd_idx] & CK_PAGE_PRESENT)) {
        /* Need to allocate a new page table */
        if (ck_page_table_count >= CK_PAGE_TABLE_COUNT) {
            return 0;  /* Out of page tables */
        }
        
        /* Use next available page table */
        uint32_t pt_addr = (uint32_t)&ck_page_tables[ck_page_table_count];
        
        /* Clear the page table */
        for (uint32_t i = 0; i < 1024; i++) {
            ck_page_tables[ck_page_table_count][i] = 0;
        }
        
        /* Create page directory entry */
        ck_page_directory[pd_idx] = ck_paging_create_pde(pt_addr, CK_PAGE_KERNEL);
        ck_page_table_count++;
        
        /* Flush TLB for this page directory entry */
        ck_paging_flush_tlb(vaddr);
    }
    
    /* Get the page table */
    ck_pte_t *pt = (ck_pte_t *)(ck_page_directory[pd_idx] & 0xFFFFF000);
    
    /* Set the page table entry */
    pt[pt_idx] = ck_paging_create_pte(paddr, flags);
    
    /* Flush TLB for this virtual address */
    ck_paging_flush_tlb(vaddr);
    
    return 1;  /* Success */
}

/*
 * Unmap a virtual page.
 */
void ck_paging_unmap_page(uint32_t vaddr) {
    uint32_t pd_idx = ck_paging_get_pd_index(vaddr);
    uint32_t pt_idx = ck_paging_get_pt_index(vaddr);
    
    if (!(ck_page_directory[pd_idx] & CK_PAGE_PRESENT)) {
        return;  /* Page table not present */
    }
    
    /* Get the page table and clear the entry */
    ck_pte_t *pt = (ck_pte_t *)(ck_page_directory[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = 0;
    
    /* Flush TLB */
    ck_paging_flush_tlb(vaddr);
}

/*
 * Get the physical address mapped to a virtual address.
 */
uint32_t ck_paging_virt_to_phys(uint32_t vaddr) {
    uint32_t pd_idx = ck_paging_get_pd_index(vaddr);
    uint32_t pt_idx = ck_paging_get_pt_index(vaddr);
    uint32_t page_offset = vaddr & 0xFFF;
    
    if (!(ck_page_directory[pd_idx] & CK_PAGE_PRESENT)) {
        return 0;  /* Not mapped */
    }
    
    ck_pte_t *pt = (ck_pte_t *)(ck_page_directory[pd_idx] & 0xFFFFF000);
    
    if (!(pt[pt_idx] & CK_PAGE_PRESENT)) {
        return 0;  /* Not mapped */
    }
    
    return (pt[pt_idx] & 0xFFFFF000) | page_offset;
}
