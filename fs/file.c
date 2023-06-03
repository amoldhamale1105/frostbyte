#include "file.h"
#include <memory/memory.h>
#include <io/print.h>
#include <lib/libc.h>
#include <debug/debug.h>
#include <process/process.h>

static struct Inode* inode_table;
static struct FileEntry* global_file_table;

static struct BPB* get_fs_bpb(void)
{
    uint32_t lba = *(uint32_t*)(FS_BASE + PARTITION_ENTRY_OFFSET + LBA_OFFSET);

    return (struct BPB*)(FS_BASE + (lba * BYTES_PER_SECTOR));
}

static uint16_t *get_fat_table(void)
{
    struct BPB* bpb = get_fs_bpb();
    uint32_t offset = (uint32_t)bpb->reserved_sector_count * bpb->bytes_per_sector;

    return (uint16_t *)((uint8_t*)bpb + offset);
}

static uint16_t get_next_cluster_index(uint32_t cluster_index)
{
    uint16_t *fat_table = get_fat_table();

    return fat_table[cluster_index];
}

static uint32_t get_cluster_size(void)
{
    struct BPB* bpb = get_fs_bpb();

    return (uint32_t)bpb->bytes_per_sector * bpb->sectors_per_cluster;
}

static uint32_t get_cluster_offset(uint32_t index)
{
    ASSERT(index >= FAT_RESERVED_BYTES);

    struct BPB* bpb = get_fs_bpb();

    /* Starting from the FAT partition, calculate the size reserved for the BIOS param block */
    uint32_t bpb_size = (uint32_t)bpb->reserved_sector_count * bpb->bytes_per_sector;
    /* Next calculate the size occupied on disk by the file allocation table section */
    uint32_t fat_size = (uint32_t)bpb->fat_count * bpb->sectors_per_fat * bpb->bytes_per_sector;
    /* Finally, calculate the size occupied by the root directory section */
    uint32_t dir_size = (uint32_t)bpb->root_entry_count * sizeof(struct DirEntry);

    /* Subtract the reserved bytes in the allocation table because the first index always starts after that */
    return bpb_size + fat_size + dir_size + (index - FAT_RESERVED_BYTES) * get_cluster_size();
}

static uint32_t get_root_dir_count(void)
{
    struct BPB* bpb = get_fs_bpb();

    return bpb->root_entry_count;
}

static struct DirEntry *get_root_dir_section(void)
{
    struct BPB* bpb = get_fs_bpb();
    /* Get the offset from FAT partition beginning (BIOS param block) to the root directory section */
    uint32_t offset = (bpb->reserved_sector_count + (uint32_t)bpb->fat_count * bpb->sectors_per_fat) * bpb->bytes_per_sector;

    return (struct DirEntry *)((uint64_t)bpb + offset);
}

static bool file_match(struct DirEntry *dir_entry, char *name, char *ext)
{
    return memcmp(dir_entry->name, name, MAX_FILENAME_BYTES) == 0 && memcmp(dir_entry->ext, ext, MAX_EXTNAME_BYTES) == 0;
}

static bool split_path(char *path, char *name, char *ext)
{
    int i;

    for (i = 0; i < MAX_FILENAME_BYTES; i++)
    {
        if (path[i] == '.' || path[i] == '\0')
            break;
        /* For now, no subdirectory paths permitted */
        if (path[i] == '/')
            return false;

        name[i] = path[i];
    }

    if (path[i] == '.') {
        i++;
        
        for (int j = 0; j < MAX_EXTNAME_BYTES; i++, j++)
        {
            if (path[i] == '\0')
                break;
            /* No subdirectories allowed */
            if (path[i] == '/')
                return false;

            ext[j] = path[i];
        }
    }

    /* After filename and extension, the pathname must be null terminated */
    if (path[i] != '\0')
        return false;

    return true;
}

static uint32_t search_file(char *path)
{
    char name[MAX_FILENAME_BYTES];
    char ext[MAX_EXTNAME_BYTES];
    uint32_t root_entry_count;
    struct DirEntry *dir_entry;
    uint32_t dir_index = DIR_ENTRY_INVALID;

    /* Initialize the buffers with spaces */
    memset(name, CHAR_SPACE_ASCII, MAX_FILENAME_BYTES);
    memset(ext, CHAR_SPACE_ASCII, MAX_EXTNAME_BYTES);

    if (split_path(path, name, ext)) {
        root_entry_count = get_root_dir_count();
        dir_entry = get_root_dir_section();

        for (uint32_t i = 0; i < root_entry_count; i++) {
            if (dir_entry[i].name[0] == ENTRY_EMPTY || dir_entry[i].name[0] == ENTRY_DELETED)
                continue;

            if (dir_entry[i].attributes == INVALID_FILETYPE)
                continue;

            if (file_match(dir_entry+i, name, ext)){
                dir_index = i;
                break;
            }
        }
    }

    return dir_index;
}

static uint32_t read_raw_data(uint32_t cluster_index, char *buf, uint32_t offset, uint32_t size)
{
    char *data;
    uint32_t read_size = 0;
    uint32_t cluster_size, start_cluster;
    uint32_t index, start_offset, copy_size, bytes_remaining;
    
    struct BPB* bpb = get_fs_bpb();
    cluster_size = get_cluster_size();
    /* Get the starting cluster number based on the file offset */
    start_cluster = offset / cluster_size;
    index = cluster_index;
    while (start_cluster)
    {
        index = get_next_cluster_index(index);
        start_cluster--;
    }

    if (index < FAT_RESERVED_BYTES)
        return UINT32_MAX;

    /* Point data to address where reading should start after calculating the offset into the chosen cluster */
    start_offset = offset % cluster_size;
    data  = (char *)((uint64_t)bpb + get_cluster_offset(index) + start_offset);
    uint32_t boundary_offset = cluster_size - start_offset;
    copy_size = size > boundary_offset ? boundary_offset : size;
    
    while (read_size < size)
    {
        memcpy(buf, data, copy_size);

        buf += copy_size;
        read_size += copy_size;
        bytes_remaining = size - read_size;
        copy_size = bytes_remaining > cluster_size ? cluster_size : bytes_remaining;

        index = get_next_cluster_index(index);
        if (index == END_OF_DATA) {
            memcpy(buf, data, bytes_remaining);
            read_size += bytes_remaining;
            break;
        }
        data  = (char *)((uint64_t)bpb + get_cluster_offset(index));
    }

    return read_size;
}

uint32_t read_file(struct Process* process, int fd, void *buf, uint32_t size)
{
    uint32_t offset = process->fd_table[fd]->offset;
    uint32_t file_size = process->fd_table[fd]->inode->file_size;

    /* Modify requested size if the offset from current position exceeds total file size */
    if (offset + size > file_size)
        size = file_size - offset;
    
    uint32_t read_size = read_raw_data(process->fd_table[fd]->inode->cluster_index, buf, offset, size);
    /* Update the file offset in global file table entry after previous read operation */
    process->fd_table[fd]->offset += read_size;

    return read_size;
}

int get_inode_entry(uint32_t dir_entry_index)
{
    struct DirEntry* dir_table;
    /* Cache the file metadata to an in core inode if it is free (ref_count == 0) */
    if (inode_table[dir_entry_index].ref_count == 0){
        dir_table = get_root_dir_section();
        /* Currently we work with a paradigm where the FAT16 root dir index is used as the in core inode table index */
        inode_table[dir_entry_index].dir_index = dir_entry_index;
        inode_table[dir_entry_index].file_size = dir_table[dir_entry_index].file_size;
        inode_table[dir_entry_index].cluster_index = dir_table[dir_entry_index].cluster_index;
        memcpy(inode_table[dir_entry_index].name, dir_table[dir_entry_index].name, MAX_FILENAME_BYTES);
        memcpy(inode_table[dir_entry_index].ext, dir_table[dir_entry_index].ext, MAX_EXTNAME_BYTES);
    }

    /* Increment the reference count of the in core inode */
    inode_table[dir_entry_index].ref_count++;

    return dir_entry_index;
}

uint32_t get_file_size(struct Process* process, int fd)
{
    return process->fd_table[fd]->inode->file_size;
}

int open_file(struct Process* process, char* pathname)
{
    int fd = -1;
    int file_table_index = -1;
    uint32_t dir_entry_index, inode_table_index;

    /* Find the first free entry in the user file descriptor table of the process */
    for(int i = 0; i < MAX_OPEN_FILES; i++)
    {
        if (process->fd_table[i] == NULL){
            fd = i;
            break;
        }
    }
    if (fd == -1)
        return fd;

    /* Next find first free entry (entry not pointing to any inode) in the global file table */
    for(int i = 0; i < PAGE_SIZE / sizeof(struct FileEntry); i++)
    {
        if (global_file_table[i].inode == NULL){
            file_table_index = i;
            break;
        }
    }
    /* If no entry available in file table, the open operation fails */
    if (file_table_index == -1)
        return -1;

    dir_entry_index = search_file(pathname);
    if (DIR_ENTRY_INVALID == dir_entry_index)
        return -1;

    inode_table_index = get_inode_entry(dir_entry_index);
    memset(global_file_table + file_table_index, 0, sizeof(struct FileEntry));
    /* An open call will always create a new file table entry. Hence we initialize the ref count to 1 */
    global_file_table[file_table_index].ref_count = 1;
    /* Link the in core inode to the global file table entry */
    global_file_table[file_table_index].inode = inode_table + inode_table_index;
    /* Link the file table entry to the process file descriptor table */
    process->fd_table[fd] = global_file_table + file_table_index;

    return fd;
}

static void inode_put(struct Inode* inode)
{
    if (inode == NULL)
        return;
    
    /* The system should halt if an iput is attempted when there are no open files */
    ASSERT(inode->ref_count > 0);
    inode->ref_count--;
    /* Release the in core inode if it's not referring to any file */
    if (inode->ref_count == 0)
        inode = NULL;
}

void close_file(struct Process* process, int fd)
{
    if (fd < 0)
        return;
    
    /* Algorithm iput => unlink the inode by decrementing reference count */
    inode_put(process->fd_table[fd]->inode);

    /* Unlink the file table entry by decrementing reference count */
    process->fd_table[fd]->ref_count--;
    /* Free the file table entry if the ref count is zero. File table entry ref count may not always be zero
       There could be occasions like a fork system call causing file table entry to be shared by the parent with the child
       This is different from the inode reference count which keeps a count of all processes accessing a file */
    if (process->fd_table[fd]->ref_count == 0)
        process->fd_table[fd] = NULL;
}

bool init_inode_table(void)
{
    inode_table = (struct Inode*)kalloc();
    if (inode_table == NULL)
        return false;

    memset(inode_table, 0, PAGE_SIZE);

    return true;
}

bool init_file_table(void)
{
    global_file_table = (struct FileEntry*)kalloc();
    if (global_file_table == NULL)
        return false;

    memset(global_file_table, 0, PAGE_SIZE);

    return true;
}

void init_fs(void)
{
    /* Get the BIOS parameter block location in the FAT16 partition derived from LBA in partition entry */
    uint8_t *bpb = (uint8_t*)get_fs_bpb();
    /* Get the value of the last 2 bytes of the BIOS parameter block sector */
    uint16_t sign = (bpb[BYTES_PER_SECTOR-1] << 8) | bpb[BYTES_PER_SECTOR-2];
    
    if (BPB_SECTOR_SIGNATURE != sign) {
        printk("Invalid FAT16 signature\n");
        ASSERT(0);
    }

    /* Setup in-core inode table and global file table */
    ASSERT(init_inode_table());
    ASSERT(init_file_table());
}

