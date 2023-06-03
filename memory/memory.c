#include "memory.h"
#include <debug/debug.h>
#include <io/print.h>
#include <lib/libc.h>
#include <fs/file.h>
#include <process/process.h>

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
    if (addr == 0)
        return;
    
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
   @param attr Memory attributes (normal or device memory)
   @return true if page mapping succeeds, false otherwise */
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

void free_page(uint64_t map, uint64_t virt_addr)
{
    uint64_t* udt_entry = NULL;
    unsigned int mdt_index;
    ASSERT(virt_addr % PAGE_SIZE == 0);

    udt_entry = find_udt_entry(map, virt_addr, 0, 0);
    if (udt_entry != NULL){
        mdt_index = (virt_addr >> 21) & 0x1ff;
        if (udt_entry[mdt_index] & ENTRY_VALID){
            kfree(TO_VIRT(PAGE_TABLE_ENTRY_ADDR(udt_entry[mdt_index])));
            /* Clear the entry indicating that it is now unused */
            udt_entry[mdt_index] = 0;
        }
    }
}

static void free_mdt(uint64_t map)
{
    uint64_t* gdt_addr = (uint64_t*)map;
    uint64_t* udt_addr = NULL;

    for(int i = 0; i < PAGE_TABLE_ENTRIES; i++)
    {
        if (gdt_addr[i] & ENTRY_VALID){
            /* Retrieve the address of the upper directory table from the GDT entry */
            udt_addr = (uint64_t*)TO_VIRT(PAGE_DIR_ENTRY_ADDR(gdt_addr[i]));
            for(int j = 0; j < PAGE_TABLE_ENTRIES; j++)
            {
                /* Free the middle directory table whose entry bit is valid in the upper directory table */
                if (udt_addr[j] & ENTRY_VALID){
                    kfree(TO_VIRT(PAGE_DIR_ENTRY_ADDR(udt_addr[j])));
                    udt_addr[j] = 0;
                }
            }
        }
    }
}

static void free_udt(uint64_t map)
{
    uint64_t* gdt_addr = (uint64_t*)map;

    for(int i = 0; i < PAGE_TABLE_ENTRIES; i++)
    {
        /* Free the upper directory table whose entry bit is valid in the global directory table */
        if (gdt_addr[i] & ENTRY_VALID){
            kfree(TO_VIRT(PAGE_DIR_ENTRY_ADDR(gdt_addr[i])));
            gdt_addr[i] = 0;
        }
    }
}

static void free_gdt(uint64_t map)
{
    kfree(map);
}

/* Function to free user space memory */
void free_vm(uint64_t map)
{
    /* User space starts at 0x400000 */
    free_page(map, USERSPACE_BASE);
    free_mdt(map);
    free_udt(map);
    free_gdt(map);
}

bool setup_uvm(struct Process* process, char* program_filename)
{
    uint64_t map = process->page_map;
    /* Allocate memory to load the process binary. We will later map it to the userspace virtual address */
    void* proc_page = kalloc();

    if (proc_page != NULL){
        memset(proc_page, 0, PAGE_SIZE);

        /* If the page mapping succeeds, load the file into that memory */
        if (map_page(map, USERSPACE_BASE, TO_PHY(proc_page), ENTRY_VALID | USER_MODE | NORMAL_MEMORY | ENTRY_ACCESSED)){
            int fd = open_file(process, program_filename);
            if (fd < 0)
                goto out;
            uint32_t binary_size = get_file_size(process, fd);
            /* Use the read_file function to load the process binary in the memory page allocated */
            if (binary_size != read_file(process, fd, proc_page, binary_size))
                goto out;
            close_file(process, fd);
            return true;
        }
    }

out:
    kfree((uint64_t)proc_page);
    free_vm(map);
    return false;
}

bool copy_uvm(uint64_t dst_map, uint64_t src_map, int size)
{
    bool status;
    uint64_t* mdt_table;
    int mdt_index;
    void* proc_page = kalloc();

    if (proc_page != NULL){
        memset(proc_page, 0, PAGE_SIZE);
        if (map_page(dst_map, USERSPACE_BASE, TO_PHY(proc_page), ENTRY_VALID | USER_MODE | NORMAL_MEMORY | ENTRY_ACCESSED)){
            /* Find the source page to copy contents to the dest page using the middle directory table
               Note that the UDT entry is nothing but address of the MDT table according to our paging setup */
            mdt_table = find_udt_entry(src_map, USERSPACE_BASE, 0, 0);
            if (mdt_table == NULL){
                status = false;
                goto err;
            }

            /* 9 bits starting from bit 21 in the virt address signify MDT table index */
            mdt_index = (USERSPACE_BASE >> 21) & 0x1ff;
            /* Check if the entry is valid. If not, it means that memory does not belong to current process */
            ASSERT((mdt_table[mdt_index] & ENTRY_VALID) == 1);
            /* Get the physical page address of the source */
            uint64_t src_mem = TO_VIRT(PAGE_TABLE_ENTRY_ADDR(mdt_table[mdt_index]));
            /* Copy the source to destination */
            memcpy(proc_page, (void*)src_mem, PAGE_SIZE);

            status = true;
            goto out;
        }
        else{
            status = false;
            goto err;
        }
    }

err:
    kfree((uint64_t)proc_page);
    free_vm(dst_map);
out:
    return status;
}

void switch_vm(uint64_t map)
{
    /* Load the TTBR0 register with global directory table address */
    load_gdt(TO_PHY(map));
}

void init_mem(void)
{
    /* Free region from end of the kernel to allocated memory end for the kernel */
    free_region((uint64_t)&kern_end, MEMORY_END);
    //checkmem();
}