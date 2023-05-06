#ifndef _FILE_H
#define _FILE_H

#include "stdint.h"

struct BPB {
    uint8_t jump[3];
    uint8_t oem[8];
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t reserved_sector_count;
    uint8_t fat_count;
    uint16_t root_entry_count;
    uint16_t sector_count;
    uint8_t media_type;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t head_count;
    uint32_t hidden_sector_count;
    uint32_t large_sector_count;
    uint8_t drive_number;
    uint8_t flags;
    uint8_t signature;
    uint32_t volume_id;
    uint8_t volume_label[11];
    uint8_t file_system[8];
} __attribute__((packed));

struct DirEntry {
    uint8_t name[8];
    uint8_t ext[3];
    uint8_t attributes;
    uint8_t reserved;
    uint8_t create_ms;
    uint16_t create_time;
    uint16_t create_date;
    uint16_t access_date;
    uint16_t attr_index;
    uint16_t m_time;
    uint16_t m_date;
    uint16_t cluster_index;
    uint32_t file_size;
} __attribute__((packed));

#define FS_BASE TO_VIRT(0x30000000)
#define BYTES_PER_SECTOR 512
#define PARTITION_ENTRY_OFFSET 0x1be
#define LBA_OFFSET 8
#define BPB_SECTOR_SIGNATURE 0xAA55
#define ENTRY_EMPTY 0
#define ENTRY_DELETED 0xe5

#define MAX_FILENAME_BYTES 8
#define MAX_EXTNAME_BYTES 3
#define INVALID_FILETYPE 15
#define DIR_ENTRY_INVALID UINT32_MAX
#define FAT_RESERVED_BYTES 2
#define END_OF_DATA 0xffff
#define CHAR_SPACE_ASCII 32

void init_fs(void);
int load_file(char *path, void* addr);

#endif