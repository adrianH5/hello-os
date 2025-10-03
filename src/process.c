#include "process.h"
#include "memory.h"

process_t* current_process = NULL;
process_t process_table[MAX_PROCESSES];
static uint32_t next_pid = 0;

void init_processes() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        process_table[i].state = PROCESS_STATE_TERMINATED;
        process_table[i].pid = 0;
        process_table[i].next = NULL;
    }
}

static process_t* allocate_pcb() {
    for (int i = 0; i < MAX_PROCESSES; i++) {
        if (process_table[i].state == PROCESS_STATE_TERMINATED) {
            return &process_table[i];
        }
    }
    return NULL;
}

process_t* create_process(void (*entry_point)()) {
    process_t* proc = allocate_pcb();
    if (!proc) {
        return NULL;  // No free PCB
    }

    proc->pid = next_pid++;
    proc->state = PROCESS_STATE_READY;

    // Clone kernel page directory for this process
    proc->page_directory = clone_directory(kernel_directory);

    // Allocate kernel stack
    proc->kernel_stack = kmalloc(PROCESS_STACK_SIZE);

    // Initialize context
    proc->context.eax = 0;
    proc->context.ebx = 0;
    proc->context.ecx = 0;
    proc->context.edx = 0;
    proc->context.esi = 0;
    proc->context.edi = 0;
    proc->context.ebp = 0;

    // Set up stack (grows downward)
    proc->context.esp = proc->kernel_stack + PROCESS_STACK_SIZE;

    // Set entry point
    proc->context.eip = (uint32_t)entry_point;

    // Set up eflags with interrupts enabled
    proc->context.eflags = 0x200;  // IF flag set

    // Set CR3 to process page directory
    proc->context.cr3 = proc->page_directory->physical_addr;

    return proc;
}

void terminate_process(process_t* proc) {
    if (!proc) {
        return;
    }

    proc->state = PROCESS_STATE_TERMINATED;
    // TODO: Free allocated memory and page directory
}

void switch_to_process(process_t* proc) {
    if (!proc || proc->state == PROCESS_STATE_TERMINATED) {
        return;
    }

    process_t* old_process = current_process;
    current_process = proc;
    proc->state = PROCESS_STATE_RUNNING;

    if (old_process) {
        old_process->state = PROCESS_STATE_READY;
        perform_context_switch(&old_process->context, &proc->context);
    } else {
        // First process, just load its context
        switch_page_directory(proc->page_directory);

        __asm__ volatile(
            "mov %0, %%esp\n"
            "mov %1, %%ebp\n"
            "mov %2, %%eax\n"
            "push %2\n"     // Push eflags
            "popf\n"        // Pop into EFLAGS
            "jmp *%3\n"
            :
            : "r"(proc->context.esp),
              "r"(proc->context.ebp),
              "r"(proc->context.eflags),
              "r"(proc->context.eip)
        );
    }
}
