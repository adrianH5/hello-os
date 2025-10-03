#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

#define PAGE_SIZE 4096
#define PAGE_ENTRIES 1024

// Page directory/table entry flags
#define PAGE_PRESENT    0x1
#define PAGE_WRITE      0x2
#define PAGE_USER       0x4

// Page directory and page table structures
typedef struct {
    uint32_t entries[PAGE_ENTRIES];
} page_table_t;

typedef struct {
    page_table_t* tables[PAGE_ENTRIES];
    uint32_t physical_tables[PAGE_ENTRIES];  // Physical addresses
    uint32_t physical_addr;                   // Physical address of this directory
} page_directory_t;

// Memory management functions
void init_paging();
void switch_page_directory(page_directory_t* dir);
page_directory_t* clone_directory(page_directory_t* src);
void map_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags);
uint32_t get_physical_address(page_directory_t* dir, uint32_t virtual_addr);

// Physical memory allocation
void init_physical_memory(uint32_t mem_size);
uint32_t alloc_frame();
void free_frame(uint32_t frame_addr);

extern page_directory_t* kernel_directory;
extern page_directory_t* current_directory;

#endif
