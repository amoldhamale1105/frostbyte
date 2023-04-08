.equ MAITR_ATTR,    (0x44 << 8) // Memory attribute indirection register, LSB 8-bits set to 0 (device memory) and next set to 0x44 (normal memory)
.equ TCR_T0SZ,      (16)
.equ TCR_T1SZ,      (16 << 16)
.equ TCR_TG0,       (0 << 14)
.equ TCR_TG1,       (2 << 30)
.equ TCR_VALUE,     (TCR_T0SZ | TCR_T1SZ | TCR_TG0 | TCR_TG1) // Translation control register value
.equ PAGE_SIZE,     (2*1024*1024) // 2M

.global enable_mmu
.global setup_vm
.global load_gdt

load_gdt:
    # Switch to userspace translation by loading ttbr0 with user space GDT address received as first parameter
    msr ttbr0_el1, x0
    # Translation Lookaside Buffer is a cache of recently accessed page translations in the MMU
    # Invalidate the translation lookaside buffer since we are switching paging from kernel to user space
    tlbi vmalle1is
    # (Data sync barrier) No instruction in program order executes until this instruction completes
    dsb ish
    # (Instruction sync barrier) Flush the pipeline in the processor so that all instructions after isb are fetched from cache or memory
    isb
    ret

enable_mmu:
    # Save addresses of the kernel and user global tables in respective ttbr system registers
    adr x0, pgd_ttbr1
    msr ttbr1_el1, x0
    
    adr x0, pgd_ttbr0
    msr ttbr0_el1, x0

    # Set the memory indirection system register
    ldr x0, =MAITR_ATTR
    msr mair_el1, x0

    # Set the TCR system register to set granule size to 4K, 48 bit virt address and 2M page size
    ldr x0, =TCR_VALUE
    msr tcr_el1, x0

    # Set bit 0 in system control register to enable paging
    mrs x0, sctlr_el1
    orr x0, x0, #1
    msr sctlr_el1, x0

setup_vm:
setup_kvm:
    # Save the kernel global and upper directory page tables
    adr x0, pgd_ttbr1
    adr x1, pud_ttbr1
    # Logical OR to set valid bit 0 and bit 1 to indicate that it points to next level page table
    orr x1, x1, #3
    # Store address of upper directory table to first entry of global directory table
    str x1, [x0]

    # Get the upper and middle directory table addresses
    adr x0, pud_ttbr1
    adr x1, pmd_ttbr1
    # Logical OR to set valid bit 0 and bit 1 to indicate that it points to next level page table
    orr x1, x1, #3
    # Save address of middle directory table to upper directory entry
    str x1, [x0]

    # Save the memory end to x2 which includes the kernel and filesystem (0x30000000 - 0x34000000) on physical memory
    mov x2, #0x34000000
    adr x1, pmd_ttbr1
    # Kernel space is mapped to virtual address space with upper 16 bits set to high (0xFFFF000000000000)
    # This is being mapped to physical address 0
    # Further we will also need to set the memmory attributes as done for user space below
    mov x0, #(1 << 10 | 1 << 2 | 1 << 0)

# Loop through all entries in the page table
loop1:
    # Store page address in middle directory table entry and increment x1 by 8
    str x0, [x1], #8
    # Point next entry to next 2M physical page
    add x0, x0, #PAGE_SIZE
    # Compare with memory end otherwise continue while it is less
    cmp x0, x2
    blo loop1

    # Map the peripheral area to the kernel space (interrupt controller, arm timer registers etc.) (0x3f000000 - 0x40000000)
    mov x2, #0x40000000
    mov x0, #0x3f000000

    adr x3, pmd_ttbr1
    # To calculate the page offset with 2M page size, shift right the start address by 21 bits (2^21 = 2M)
    # Each table entry being 8 bytes, we shift left by 3 bits to get correct offset (left shifting by 3 is equivalent to multiplying by 8)
    # Store the entry offset in x1
    lsr x1, x0, #(21 - 3)
    # Add the offset to the table address
    add x1, x1, x3
    # Set the memory attributes, clear bit 2 to set the index value to 0 which stands for device memory
    orr x0, x0, #1
    orr x0, x0, #(1 << 10)

# Loop through all entries for the device memory
loop2:
    # Store page address in middle directory table entry and increment x1 by 8
    str x0, [x1], #8
    # Point next entry to next 2M physical page
    add x0, x0, #PAGE_SIZE
    # Compare with memory end otherwise continue while it is less
    cmp x0, x2
    blo loop2

    # Map addresses greater than 1G in the second entry of the upper directory table (first entry mapped the lower 1G)
    adr x0, pud_ttbr1
    # Add 8 to upper directory table to get seond entry
    add x0, x0, #(1 * 8)
    adr x1, pmd2_ttbr1
    orr x1, x1, #3
    str x1, [x0]

    # Start and end address of memory mapped registers (0x40000000 - 0x41000000)
    mov x2, #0x41000000
    mov x0, #0x40000000

    adr x1, pmd2_ttbr1
    orr x0, x0, #1
    orr x0, x0, #(1 << 10)

# Loop through all entries for memory mapped registers above 1G
loop3:
    # Store page address in middle directory table entry and increment x1 by 8
    str x0, [x1], #8
    # Point next entry to next 2M physical page
    add x0, x0, #PAGE_SIZE
    # Compare with memory end otherwise continue while it is less
    cmp x0, x2
    blo loop3

setup_uvm:
    # Get the addresses of the global and upper page directory tables
    adr x0, pgd_ttbr0
    adr x1, pud_ttbr0
    # Logical OR to set valid bit 0 and bit 1 to indicate that it points to next level page table
    orr x1, x1, #3
    # Store address of upper directory table to first entry of global directory table
    str x1, [x0]

    # Get the upper and middle directory table addresses
    adr x0, pud_ttbr0
    adr x1, pmd_ttbr0
    # Logical OR to set valid bit 0 and bit 1 to indicate that it points to next level page table
    orr x1, x1, #3
    # Save address of middle directory table to upper directory entry
    str x1, [x0]

    # Get the address of the middle directory table
    adr x1, pmd_ttbr0
    # Set valid bit (bit 0) to 1, access bit (bit 10) to 1
    # Bits 2-4 are an index to the memory attribute indirection register. We set bit 2 to high so that the index value will be 1
    mov x0, #(1 << 10 | 1 << 2 | 1 << 0)
    # Save the value to first entry of the middle directory table
    str x0, [x1]

    ret

.balign 4096
# Kernel mode translation tables
# Top level global page directory table with 512 entries, 8-byte each. Each entry represents 512G of memory
pgd_ttbr1:
    # Allocate 4k of storage filled with zeros
    .space 4096
# Upper directory table of 4k size. Each entry in this table represents 1G of memory
pud_ttbr1:
    .space 4096
# Middle directory table. Every entry in this table points to 2M physical page
pmd_ttbr1:
    .space 4096
# Middle directory to map addresses beyomd the first 1G physical memory chunk
pmd2_ttbr1:
    .space 4096

# User mode translation tables
# Top level global page directory table with 512 entries, 8-byte each. Each entry represents 512G of memory
pgd_ttbr0:
    # Allocate 4k of storage filled with zeros
    .space 4096
# Upper directory table of 4k size. Each entry in this table represents 1G of memory
pud_ttbr0:
    .space 4096
# Middle directory table. Every entry in this table points to 2M physical page
pmd_ttbr0:
    .space 4096
