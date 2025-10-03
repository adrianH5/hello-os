#include "scheduler.h"
#include "process.h"

static process_t* ready_queue_head = NULL;
static process_t* ready_queue_tail = NULL;

void init_scheduler() {
    ready_queue_head = NULL;
    ready_queue_tail = NULL;
}

void add_to_ready_queue(process_t* proc) {
    if (!proc) {
        return;
    }

    proc->state = PROCESS_STATE_READY;
    proc->next = NULL;

    if (!ready_queue_head) {
        ready_queue_head = proc;
        ready_queue_tail = proc;
    } else {
        ready_queue_tail->next = proc;
        ready_queue_tail = proc;
    }
}

void schedule() {
    // Simple round-robin scheduler
    if (!ready_queue_head) {
        return;  // No processes to schedule
    }

    // Get next process from queue
    process_t* next_process = ready_queue_head;
    ready_queue_head = next_process->next;

    if (!ready_queue_head) {
        ready_queue_tail = NULL;
    }

    // Add current process back to queue if it's still ready/running
    if (current_process && current_process->state != PROCESS_STATE_TERMINATED) {
        add_to_ready_queue(current_process);
    }

    // Switch to next process
    switch_to_process(next_process);
}
