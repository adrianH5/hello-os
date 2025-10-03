#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "process.h"

// Scheduler functions
void init_scheduler();
void schedule();
void add_to_ready_queue(process_t* proc);

#endif
