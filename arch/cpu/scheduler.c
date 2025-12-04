#include "arch/cpu/scheduler.h"
#include <arch/bsp/uart.h>
#include <arch/cpu/interrupts.h>
#include <lib/kprintf.h>
#include <lib/mem.h>

// Thread table
static tcb_t thread_table[MAX_THREADS];
static uint32_t current_thread_id = 0;
static bool scheduler_running = false;

// Idle thread function
static void idle_thread(void *arg) {
    (void)arg;
    while (1) {
        __asm volatile("wfi"); // Wait for interrupt
    }
}

static void syscall_exit(void) {
    __asm volatile("svc #0");
    // Never returns
    while(1);
}

// Thread wrapper - calls thread function and exits when done
static void thread_wrapper(void) {
    tcb_t *current = &thread_table[current_thread_id];
    
    // Get the actual thread function and argument from the stack
    void (*func)(void *) = (void (*)(void *))current->context.r0;
    void *arg = (void *)current->context.r1;
    
    // Call the thread function
    func(arg);
    
    // Thread finished - exit
    syscall_exit();
}

void scheduler_init(void) {
    // Initialize all threads as terminated
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_table[i].state = THREAD_STATE_TERMINATED;
        thread_table[i].thread_id = i;
    }
    
    // Create idle thread (thread 0)
    current_thread_id = 0;
    thread_table[0].state = THREAD_STATE_RUNNING;
    thread_table[0].context.sp = (uint32_t)&thread_table[0].stack[THREAD_STACK_SIZE - 4];
    thread_table[0].context.pc = (uint32_t)idle_thread;
    thread_table[0].context.lr = 0;
    thread_table[0].context.cpsr = 0x10; // User mode, all interrupts enabled
    
    scheduler_running = false;
}

void scheduler_start(void) {
    scheduler_running = true;
    
    // Select the first thread to run
    scheduler_schedule();
    
    // Get the selected thread
    tcb_t *thread = &thread_table[current_thread_id];
    
    // Jump to the thread - this never returns
    __asm volatile(
        "mov sp, %0\n"           // Set stack pointer
        "mov lr, %1\n"           // Set return address (PC)
        "msr cpsr_c, %2\n"       // Set CPSR (switch to user mode)
        "mov pc, lr\n"           // Jump to thread
        :
        : "r"(thread->context.sp),
          "r"(thread->context.pc),
          "r"(thread->context.cpsr)
        : "memory"
    );
    
    // Should never reach here
    __builtin_unreachable();
}

void scheduler_thread_create(void (*func)(void *), const void *arg, unsigned int arg_size) {
    // Disable interrupts during thread creation
    uint32_t cpsr;
    __asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    __asm volatile("cpsid if");
    
    // Find free thread slot
    int free_slot = -1;
    for (int i = 1; i < MAX_THREADS; i++) { // Start at 1 (skip idle thread)
        if (thread_table[i].state == THREAD_STATE_TERMINATED) {
            free_slot = i;
            break;
        }
    }
    
    if (free_slot == -1) {
        uart_puts("Could not create thread.\n");
        __asm volatile("msr cpsr_c, %0" : : "r"(cpsr));
        return;
    }
    
    tcb_t *new_thread = &thread_table[free_slot];
    
    // Initialize stack (grows downward)
    uint32_t *stack_ptr = (uint32_t *)&new_thread->stack[THREAD_STACK_SIZE];
    
    // Copy argument to thread stack if provided
    void *arg_ptr = NULL;
    if (arg && arg_size > 0) {
        stack_ptr = (uint32_t *)((uint8_t *)stack_ptr - arg_size);
        // Align to 4 bytes
        stack_ptr = (uint32_t *)((uint32_t)stack_ptr & ~0x3);
        memcpy(stack_ptr, arg, arg_size);
        arg_ptr = stack_ptr;
    }
    
    // Reserve space for initial context (16 words = 64 bytes)
    stack_ptr -= 16;
    
    // Initialize thread context
    memset(&new_thread->context, 0, sizeof(thread_context_t));
    new_thread->context.r0 = (uint32_t)func;        // Function pointer
    new_thread->context.r1 = (uint32_t)arg_ptr;     // Argument pointer
    new_thread->context.sp = (uint32_t)stack_ptr;
    new_thread->context.pc = (uint32_t)thread_wrapper;
    new_thread->context.lr = 0;
    new_thread->context.cpsr = 0x10; // User mode (0x10), IRQ enabled
    
    new_thread->state = THREAD_STATE_READY;
    new_thread->thread_id = free_slot;
    
    // Restore interrupts
    __asm volatile("msr cpsr_c, %0" : : "r"(cpsr));
}

void scheduler_schedule(void) {
    if (!scheduler_running) {
        return;
    }
    
    // Print newline for context switch (as per requirements)
    uart_putc('\n');
    
    // Save current thread state (if running)
    if (thread_table[current_thread_id].state == THREAD_STATE_RUNNING) {
        thread_table[current_thread_id].state = THREAD_STATE_READY;
    }
    
    // Round-robin: find next ready thread
    uint32_t start_id = current_thread_id;
    uint32_t next_id = (current_thread_id + 1) % MAX_THREADS;
    
    while (next_id != start_id) {
        if (thread_table[next_id].state == THREAD_STATE_READY) {
            current_thread_id = next_id;
            thread_table[current_thread_id].state = THREAD_STATE_RUNNING;
            return;
        }
        next_id = (next_id + 1) % MAX_THREADS;
    }
    
    // No ready thread found - check current thread
    if (thread_table[current_thread_id].state == THREAD_STATE_READY) {
        thread_table[current_thread_id].state = THREAD_STATE_RUNNING;
        return;
    }
    
    // Fall back to idle thread (thread 0)
    current_thread_id = IDLE_THREAD_ID;
    thread_table[current_thread_id].state = THREAD_STATE_RUNNING;
}

void scheduler_terminate_current_thread(void) {
    // Mark current thread as terminated
    thread_table[current_thread_id].state = THREAD_STATE_TERMINATED;
    
    // Schedule next thread
    scheduler_schedule();
}

tcb_t* scheduler_get_current_thread(void) {
    return &thread_table[current_thread_id];
}

// Context switch function - called from IRQ handler
void scheduler_context_switch(exc_frame_t *frame) {
    if (!scheduler_running) {
        return;
    }
    
    // Save current thread context from exception frame
    tcb_t *current = &thread_table[current_thread_id];
    current->context.r0 = frame->r0;
    current->context.r1 = frame->r1;
    current->context.r2 = frame->r2;
    current->context.r3 = frame->r3;
    current->context.r4 = frame->r4;
    current->context.r5 = frame->r5;
    current->context.r6 = frame->r6;
    current->context.r7 = frame->r7;
    current->context.r8 = frame->r8;
    current->context.r9 = frame->r9;
    current->context.r10 = frame->r10;
    current->context.r11 = frame->r11;
    current->context.r12 = frame->r12;
    current->context.sp = frame->sp;
    current->context.lr = frame->lr;
    current->context.pc = frame->lr; // PC is the return address (saved as LR in the frame)
    current->context.cpsr = frame->spsr;
    
    // Select next thread
    scheduler_schedule();
    
    // Restore next thread context to exception frame
    tcb_t *next = &thread_table[current_thread_id];
    frame->r0 = next->context.r0;
    frame->r1 = next->context.r1;
    frame->r2 = next->context.r2;
    frame->r3 = next->context.r3;
    frame->r4 = next->context.r4;
    frame->r5 = next->context.r5;
    frame->r6 = next->context.r6;
    frame->r7 = next->context.r7;
    frame->r8 = next->context.r8;
    frame->r9 = next->context.r9;
    frame->r10 = next->context.r10;
    frame->r11 = next->context.r11;
    frame->r12 = next->context.r12;
    frame->sp = next->context.sp;
    frame->lr = next->context.pc; // Restore PC into the saved LR register
    frame->spsr = next->context.cpsr;
}
