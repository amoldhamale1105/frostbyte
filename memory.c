#include "memory.h"
#include "debug.h"
#include "print.h"
#include "libc.h"

static struct Page free_mem_head = {
    .next = NULL
};
/* The symbol used in linker script whose address will mark the end of kernel in the virt address space */
extern char kern_end;
void load_gdt(uint64_t map);

static void free_region(uint64_t start, uint64_t end)
{
    for(uint64_t addr = ALIGN_UP(start); addr + PAGE_SIZE <= end; addr += PAGE_SIZE)
    {
        /* If page is within the region, add it to list of free pages */
        if (addr + PAGE_SIZE <= MEMORY_END)
            kfree(addr);
    }
}

void *kalloc(void)
{
    struct Page* page = free_mem_head.next;
    
    if (page != NULL){
        /* Assert that the virtual address is page aligned */
        ASSERT((uint64_t)page % PAGE_SIZE == 0);
        /* Assert that the virtual address is not within kernel space */
        ASSERT((uint64_t)page >= (uint64_t)&kern_end);
        /* Assert that the address is within memory limit */
        ASSERT((uint64_t)page + PAGE_SIZE <= MEMORY_END);

        free_mem_head.next = page->next;
    }
    
    return page;
}

/* A test function to print the list of free memory pages and total size in megabytes */
static void checkmem(void)
{
    /* Get the first page in the list */
    struct Page* page = free_mem_head.next;
    uint64_t size = 0;
    int index = 0;

    while (page != NULL)
    {
        size += PAGE_SIZE;
        index++;

        printk("Page number %d: %x\r\n", index, (uint64_t)page);
        page = page->next;
    }

    printk("Total free mem: %uM\r\n", size/PAGE_SIZE*2);
}

void kfree(uint64_t addr)
{
    /* Assert that the virtual address is page aligned */
    ASSERT(addr % PAGE_SIZE == 0);
    /* Assert that the virtual address is not within kernel space */
    ASSERT(addr >= (uint64_t)&kern_end);
    /* Assert that the address is within memory limit */
    ASSERT(addr + PAGE_SIZE <= MEMORY_END);

    /* Add the page to be freed to the linked list of free pages just after the head */
    struct Page* page_addr = (struct Page*)addr;
    page_addr->next = free_mem_head.next;
    free_mem_head.next = page_addr;
}

static uint64_t* find_gdt_entry(uint64_t map, uint64_t virt_addr, int alloc_new, uint64_t attr)
{
    uint64_t* gdt_addr = (uint64_t*)map;
    uint64_t* gdt_entry = NULL;

    /* 9 bits after lower 39 bits of the virt address signify index into the GDT table */
    unsigned int gdt_index = (virt_addr >> 39) & 0x1ff;

    if (gdt_addr == NULL)
        return NULL;

    if (gdt_addr[gdt_index] & ENTRY_VALID)
        gdt_entry = (uint64_t*)(TO_VIRT(PAGE_DIR_ENTRY_ADDR(gdt_addr[gdt_index])));
    else if (alloc_new){
        gdt_entry = kalloc();
        if (gdt_entry != NULL){
            /* Initialize the upper directory table to all zeros */
            memset(gdt_entry, 0, PAGE_SIZE);
            gdt_addr[gdt_index] = (TO_PHY(gdt_entry) | attr | TABLE_ENTRY);
        }
    }

    return gdt_entry;
}

static uint64_t* find_udt_entry(uint64_t map, uint64_t virt_addr, int alloc_new, uint64_t attr)
{
    uint64_t* gdt_entry, *udt_entry;
    gdt_entry = udt_entry = NULL;

    /* 9 bits after 30 bits in LSB holds the index for the upper directory table */
    unsigned int udt_index = (virt_addr >> 30) & 0x1ff;

    if (NULL == (gdt_entry = find_gdt_entry(map, virt_addr, alloc_new, attr)))
        return NULL;

    /* For a page directory table if valid bit in clear, the entry is unused */
    if (gdt_entry[udt_index] & ENTRY_VALID)
        udt_entry = (uint64_t*)TO_VIRT(PAGE_DIR_ENTRY_ADDR(gdt_entry[udt_index]));
    /* If alloc_new is 1, allocate a new page if it does not exist */
    else if (alloc_new){
        udt_entry = kalloc();
        if (udt_entry != NULL){
            /* Initialize the middle directory table to all zeros */
            memset(udt_entry, 0, PAGE_SIZE);
            gdt_entry[udt_index] = (TO_PHY(udt_entry) | attr | TABLE_ENTRY);
        }
    }

    return udt_entry;
}

/* Map virtual address to corresponding physical page
   @param map Global directory table address value
   @param virt_addr virtual address to be mapped
   @param phy_addr physical address mapped to
   @param attr Memory attributes (normal or device memory) */
bool map_page(uint64_t map, uint64_t virt_addr, uint64_t phy_addr, uint64_t attr)
{
    /* Get the beginning of the page in which this virtual address falls */
    uint64_t vstart = ALIGN_DOWN(virt_addr);
    uint64_t* udt_entry = NULL;

    ASSERT(vstart + PAGE_SIZE <= MEMORY_END);
    ASSERT(phy_addr % PAGE_SIZE == 0);
    /* Check if physical address falls outside range of free memory */
    ASSERT(phy_addr + PAGE_SIZE <= TO_PHY(MEMORY_END));

    /* Get the upper directory table entry corresponding to the virtual address start */
    if (NULL == (udt_entry = find_udt_entry(map, vstart, 1, attr)))
        return false;

    /* 9 bits just ahead of last 21 bits in the virt address hold the MDT entry index */
    unsigned int mdt_index = (vstart >> 21) & 0x1ff;
    /* Check if valid bit is set, which imples page is already used */
    ASSERT((udt_entry[mdt_index] & ENTRY_VALID) == 0);

    udt_entry[mdt_index] = (phy_addr | attr | PAGE_ENTRY);

    return true;
}

bool setup_uvm(void)
{
    return false;
}

bool switch_vm(uint64_t map)
{
    /* Load the TTBR0 register with global directory table address */
    load_gdt(TO_PHY(map));
    return false;
}

void init_mem(void)
{
    /* Free region from end of the kernel to allocated memory end for the kernel */
    free_region((uint64_t)&kern_end, MEMORY_END);
    checkmem();
}