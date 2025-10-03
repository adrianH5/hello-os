#ifndef PROCESS_H
#define PROCESS_H

#include "types.h"
#include "memory.h"

#define MAX_PROCESSES 32
#define PROCESS_STACK_SIZE 4096

typedef enum {
    PROCESS_STATE_READY,
    PROCESS_STATE_RUNNING,
    PROCESS_STATE_BLOCKED,
    PROCESS_STATE_TERMINATED
} process_state_t;

// CPU context saved during context switch
typedef struct {
    uint32_t eax, ebx, ecx, edx;
    uint32_t esi, edi, ebp;
    uint32_t esp;
    uint32_t eip;
    uint32_t eflags;
    uint32_t cr3;  // Page directory
} cpu_context_t;

// Process Control Block (PCB)
typedef struct process {
    uint32_t pid;
    process_state_t state;
    cpu_context_t context;
    page_directory_t* page_directory;
    uint32_t kernel_stack;
    struct process* next;  // For ready queue
} process_t;

// Process management functions
void init_processes();
process_t* create_process(void (*entry_point)());
void terminate_process(process_t* proc);
void switch_to_process(process_t* proc);

// Context switching (implemented in assembly)
extern void perform_context_switch(cpu_context_t* old_context, cpu_context_t* new_context);

extern process_t* current_process;
extern process_t process_table[MAX_PROCESSES];

#endif
