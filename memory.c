#include "memory.h"
#include "debug.h"
#include "print.h"

static struct Page free_mem_head = {
    .next = NULL
};
/* The symbol used in linker script whose address will mark the end of kernel in the virt address space */
extern char kern_end;

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
        /* Asset that the virtual address is not within kernel space */
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
    /* Asset that the virtual address is not within kernel space */
    ASSERT(addr >= (uint64_t)&kern_end);
    /* Assert that the address is within memory limit */
    ASSERT(addr + PAGE_SIZE <= MEMORY_END);

    /* Add the page to be freed to the linked list of free pages just after the head */
    struct Page* page_addr = (struct Page*)addr;
    page_addr->next = free_mem_head.next;
    free_mem_head.next = page_addr;
}

void init_mem(void)
{
    /* Free region from end of the kernel to allocated memory end for the kernel */
    free_region((uint64_t)&kern_end, MEMORY_END);
    checkmem();
}