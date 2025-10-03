#include "types.h"
#include "memory.h"
#include "process.h"
#include "scheduler.h"

// Video memory
#define VIDEO_MEMORY 0xb8000
#define WHITE_ON_BLACK 0x0f

static char* video_memory = (char*)VIDEO_MEMORY;
static int cursor_x = 0;
static int cursor_y = 0;

void clear_screen() {
    for (int i = 0; i < 80 * 25 * 2; i++) {
        video_memory[i] = 0;
    }
    cursor_x = 0;
    cursor_y = 0;
}

void print(const char* str) {
    while (*str) {
        if (*str == '\n') {
            cursor_x = 0;
            cursor_y++;
        } else {
            int offset = (cursor_y * 80 + cursor_x) * 2;
            video_memory[offset] = *str;
            video_memory[offset + 1] = WHITE_ON_BLACK;
            cursor_x++;
            if (cursor_x >= 80) {
                cursor_x = 0;
                cursor_y++;
            }
        }
        str++;
    }
}

void print_hex(uint32_t n) {
    char hex_chars[] = "0123456789ABCDEF";
    char buf[11];
    buf[0] = '0';
    buf[1] = 'x';
    buf[10] = 0;

    for (int i = 9; i >= 2; i--) {
        buf[i] = hex_chars[n & 0xF];
        n >>= 4;
    }

    print(buf);
}

// Example process functions
void process1() {
    int count = 0;
    while (1) {
        // Process 1 work
        count++;

        // Simple delay
        for (volatile int i = 0; i < 10000000; i++);

        // Yield to scheduler
        schedule();
    }
}

void process2() {
    int count = 0;
    while (1) {
        // Process 2 work
        count++;

        // Simple delay
        for (volatile int i = 0; i < 10000000; i++);

        // Yield to scheduler
        schedule();
    }
}

void process3() {
    int count = 0;
    while (1) {
        // Process 3 work
        count++;

        // Simple delay
        for (volatile int i = 0; i < 10000000; i++);

        // Yield to scheduler
        schedule();
    }
}

void kernel_main() {
    clear_screen();

    print("OS Booting...\n");
    print("Initializing memory virtualization...\n");

    // Initialize paging
    init_paging();
    print("Paging enabled\n");

    // Initialize process management
    init_processes();
    print("Process management initialized\n");

    // Initialize scheduler
    init_scheduler();
    print("Scheduler initialized\n");

    print("\nCreating processes...\n");

    // Create some processes
    process_t* p1 = create_process(process1);
    if (p1) {
        print("Process 1 created (PID: ");
        print_hex(p1->pid);
        print(")\n");
        add_to_ready_queue(p1);
    }

    process_t* p2 = create_process(process2);
    if (p2) {
        print("Process 2 created (PID: ");
        print_hex(p2->pid);
        print(")\n");
        add_to_ready_queue(p2);
    }

    process_t* p3 = create_process(process3);
    if (p3) {
        print("Process 3 created (PID: ");
        print_hex(p3->pid);
        print(")\n");
        add_to_ready_queue(p3);
    }

    print("\nStarting scheduler...\n");
    print("Processes are now running!\n");

    // Start scheduling
    schedule();

    // Should never reach here
    print("Kernel panic: Scheduler returned!\n");
    while (1);
}
