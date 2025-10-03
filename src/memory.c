#include "memory.h"
#include "types.h"

// Bitmap for physical frame allocation
static uint32_t* frames;
static uint32_t nframes;

page_directory_t* kernel_directory = 0;
page_directory_t* current_directory = 0;

// Simple memory allocator (placement address)
static uint32_t placement_address = 0x100000;  // Start at 1MB

static uint32_t kmalloc_internal(uint32_t size, int align, uint32_t* phys) {
    if (align && (placement_address & 0xFFF)) {
        placement_address &= 0xFFFFF000;
        placement_address += 0x1000;
    }

    if (phys) {
        *phys = placement_address;
    }

    uint32_t tmp = placement_address;
    placement_address += size;
    return tmp;
}

uint32_t kmalloc(uint32_t size) {
    return kmalloc_internal(size, 0, 0);
}

uint32_t kmalloc_a(uint32_t size) {
    return kmalloc_internal(size, 1, 0);
}

uint32_t kmalloc_p(uint32_t size, uint32_t* phys) {
    return kmalloc_internal(size, 0, phys);
}

uint32_t kmalloc_ap(uint32_t size, uint32_t* phys) {
    return kmalloc_internal(size, 1, phys);
}

// Frame allocation functions
static void set_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frames[idx] |= (1 << off);
}

static void clear_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    frames[idx] &= ~(1 << off);
}

static uint32_t test_frame(uint32_t frame_addr) {
    uint32_t frame = frame_addr / PAGE_SIZE;
    uint32_t idx = frame / 32;
    uint32_t off = frame % 32;
    return (frames[idx] & (1 << off));
}

static uint32_t first_free_frame() {
    for (uint32_t i = 0; i < nframes / 32; i++) {
        if (frames[i] != 0xFFFFFFFF) {
            for (uint32_t j = 0; j < 32; j++) {
                if (!(frames[i] & (1 << j))) {
                    return i * 32 + j;
                }
            }
        }
    }
    return (uint32_t)-1;
}

void init_physical_memory(uint32_t mem_size) {
    nframes = mem_size / PAGE_SIZE;
    frames = (uint32_t*)kmalloc(nframes / 8);

    // Clear all frames
    for (uint32_t i = 0; i < nframes / 32; i++) {
        frames[i] = 0;
    }
}

uint32_t alloc_frame() {
    uint32_t idx = first_free_frame();
    if (idx == (uint32_t)-1) {
        return 0;  // No free frames
    }
    set_frame(idx * PAGE_SIZE);
    return idx * PAGE_SIZE;
}

void free_frame(uint32_t frame_addr) {
    clear_frame(frame_addr);
}

// Paging functions
void switch_page_directory(page_directory_t* dir) {
    current_directory = dir;

    // Load page directory into CR3
    __asm__ volatile("mov %0, %%cr3" :: "r"(dir->physical_addr));

    // Enable paging
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;  // Set PG bit
    __asm__ volatile("mov %0, %%cr0" :: "r"(cr0));
}

void map_page(page_directory_t* dir, uint32_t virtual_addr, uint32_t physical_addr, uint32_t flags) {
    uint32_t page_dir_idx = virtual_addr >> 22;
    uint32_t page_table_idx = (virtual_addr >> 12) & 0x3FF;

    if (dir->tables[page_dir_idx] == 0) {
        // Create page table
        uint32_t phys;
        dir->tables[page_dir_idx] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &phys);

        // Clear the table
        for (int i = 0; i < PAGE_ENTRIES; i++) {
            dir->tables[page_dir_idx]->entries[i] = 0;
        }

        dir->physical_tables[page_dir_idx] = phys | PAGE_PRESENT | PAGE_WRITE | flags;
    }

    dir->tables[page_dir_idx]->entries[page_table_idx] = physical_addr | flags | PAGE_PRESENT;
}

void init_paging() {
    // Initialize physical memory (assuming 16MB)
    init_physical_memory(0x1000000);

    // Create kernel page directory
    uint32_t phys;
    kernel_directory = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);

    // Initialize to zeros
    for (int i = 0; i < PAGE_ENTRIES; i++) {
        kernel_directory->tables[i] = 0;
        kernel_directory->physical_tables[i] = 0;
    }
    kernel_directory->physical_addr = phys;

    // Identity map first 4MB (kernel space)
    for (uint32_t i = 0; i < 0x400000; i += PAGE_SIZE) {
        map_page(kernel_directory, i, i, PAGE_PRESENT | PAGE_WRITE);
    }

    // Switch to our new page directory
    switch_page_directory(kernel_directory);
}

page_directory_t* clone_directory(page_directory_t* src) {
    uint32_t phys;
    page_directory_t* dir = (page_directory_t*)kmalloc_ap(sizeof(page_directory_t), &phys);
    dir->physical_addr = phys;

    for (int i = 0; i < PAGE_ENTRIES; i++) {
        if (!src->tables[i]) {
            dir->tables[i] = 0;
            dir->physical_tables[i] = 0;
        } else {
            // Clone the page table
            uint32_t table_phys;
            dir->tables[i] = (page_table_t*)kmalloc_ap(sizeof(page_table_t), &table_phys);

            for (int j = 0; j < PAGE_ENTRIES; j++) {
                if (src->tables[i]->entries[j] & PAGE_PRESENT) {
                    // Allocate new frame for user pages
                    if (src->tables[i]->entries[j] & PAGE_USER) {
                        uint32_t new_frame = alloc_frame();
                        dir->tables[i]->entries[j] = new_frame | (src->tables[i]->entries[j] & 0xFFF);

                        // Copy page contents
                        uint32_t* src_page = (uint32_t*)(src->tables[i]->entries[j] & 0xFFFFF000);
                        uint32_t* dst_page = (uint32_t*)new_frame;
                        for (int k = 0; k < PAGE_SIZE / 4; k++) {
                            dst_page[k] = src_page[k];
                        }
                    } else {
                        // Share kernel pages
                        dir->tables[i]->entries[j] = src->tables[i]->entries[j];
                    }
                } else {
                    dir->tables[i]->entries[j] = 0;
                }
            }

            dir->physical_tables[i] = table_phys | (src->physical_tables[i] & 0xFFF);
        }
    }

    return dir;
}

uint32_t get_physical_address(page_directory_t* dir, uint32_t virtual_addr) {
    uint32_t page_dir_idx = virtual_addr >> 22;
    uint32_t page_table_idx = (virtual_addr >> 12) & 0x3FF;
    uint32_t offset = virtual_addr & 0xFFF;

    if (dir->tables[page_dir_idx]) {
        uint32_t entry = dir->tables[page_dir_idx]->entries[page_table_idx];
        if (entry & PAGE_PRESENT) {
            return (entry & 0xFFFFF000) + offset;
        }
    }

    return 0;
}
