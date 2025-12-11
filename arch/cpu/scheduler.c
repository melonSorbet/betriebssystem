#include "arch/cpu/scheduler.h"
#include <arch/bsp/uart.h>
#include <arch/cpu/interrupts.h>
#include <lib/kprintf.h>
#include <lib/mem.h>

static tcb_t thread_table[MAX_THREADS];
static uint32_t current_thread_id = 0;
static bool scheduler_running = false;

static void idle_thread(void *arg) {
    (void)arg;
    while (1) {
    }
}

void syscall_exit(void) {
    __asm volatile("svc #0");
    while(1);
}

static void thread_wrapper(void) {
    uint32_t func_ptr, arg_ptr;
    
    __asm volatile(
        "mov %0, r0\n"
        "mov %1, r1\n"
        : "=r"(func_ptr), "=r"(arg_ptr)
        :
        : "r0", "r1"
    );
    
    void (*func)(void *) = (void (*)(void *))func_ptr;
    void *arg = (void *)arg_ptr;
    
    if (arg) {
    }
    
    func(arg);
    
    syscall_exit();
}

void scheduler_init(void) {
    for (int i = 0; i < MAX_THREADS; i++) {
        thread_table[i].state = THREAD_STATE_TERMINATED;
        thread_table[i].thread_id = i;
    }
    
    current_thread_id = 0;
    thread_table[0].state = THREAD_STATE_RUNNING;
    
    memset(&thread_table[0].context, 0, sizeof(thread_context_t));
    
    uint32_t stack_top = (uint32_t)&thread_table[0].stack[THREAD_STACK_SIZE];
    stack_top &= ~0x7;
    
    thread_table[0].context.sp = stack_top;
    thread_table[0].context.lr = (uint32_t)idle_thread;
    thread_table[0].context.cpsr = 0x10;
    
    scheduler_running = false;
}

void scheduler_start [[noreturn]] (void) {
    scheduler_running = true;
    while (1) {
    
    }
    
    __builtin_unreachable();
}

void scheduler_thread_create(void (*func)(void *), const void *arg, unsigned int arg_size) {
    uint32_t cpsr;
    __asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    __asm volatile("cpsid if");
    
    int free_slot = -1;
    for (int i = 1; i < MAX_THREADS; i++) {
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
    
    uint32_t stack_top = (uint32_t)&new_thread->stack[THREAD_STACK_SIZE];
    stack_top &= ~0x7;
    
    void *arg_ptr = NULL;
    if (arg && arg_size > 0) {
        unsigned int aligned_size = (arg_size + 7) & ~0x7;
        stack_top -= aligned_size;
        memcpy((void *)stack_top, arg, arg_size);
        arg_ptr = (void *)stack_top;
    }
    
    memset(&new_thread->context, 0, sizeof(thread_context_t));
    
    new_thread->context.r0 = (uint32_t)func;
    new_thread->context.r1 = (uint32_t)arg_ptr;
    new_thread->context.sp = stack_top;
    new_thread->context.lr = (uint32_t)thread_wrapper;
    new_thread->context.cpsr = 0x10;
    
    new_thread->state = THREAD_STATE_READY;
    new_thread->thread_id = free_slot;
    
    __asm volatile("msr cpsr_c, %0" : : "r"(cpsr));
}

void scheduler_schedule(void) {
    if (!scheduler_running) {
        return;
    }
    
    uint32_t old_thread_id = current_thread_id;
    
    if (thread_table[current_thread_id].state == THREAD_STATE_RUNNING) {
        thread_table[current_thread_id].state = THREAD_STATE_READY;
    }
    
    uint32_t next_id = current_thread_id;
    
    for (uint32_t count = 0; count < MAX_THREADS - 1; count++) {
        if (next_id == 0 || next_id >= MAX_THREADS - 1) {
            next_id = 1;
        } else {
            next_id++;
        }
        
        if (thread_table[next_id].state == THREAD_STATE_READY) {
            current_thread_id = next_id;
            thread_table[current_thread_id].state = THREAD_STATE_RUNNING;
            
            if (old_thread_id != current_thread_id) {
                uart_putc('\n');
            }
            return;
        }
    }
    
    current_thread_id = IDLE_THREAD_ID;
    thread_table[IDLE_THREAD_ID].state = THREAD_STATE_RUNNING;
}

void scheduler_terminate_current_thread(void) {
    thread_table[current_thread_id].state = THREAD_STATE_TERMINATED;
}

void scheduler_context_switch(exc_frame_t *frame) {
    if (!scheduler_running) {
        return;
    }

    tcb_t *current = &thread_table[current_thread_id];
    uint32_t old_thread_id = current_thread_id;

    scheduler_schedule();

    if (old_thread_id != current_thread_id && current->state != THREAD_STATE_TERMINATED) {
        current->context.cpsr = frame->spsr;
        current->context.sp   = frame->sp;
        current->context.r0  = frame->r0;
        current->context.r1  = frame->r1;
        current->context.r2  = frame->r2;
        current->context.r3  = frame->r3;
        current->context.r4  = frame->r4;
        current->context.r5  = frame->r5;
        current->context.r6  = frame->r6;
        current->context.r7  = frame->r7;
        current->context.r8  = frame->r8;
        current->context.r9  = frame->r9;
        current->context.r10 = frame->r10;
        current->context.r11 = frame->r11;
        current->context.r12 = frame->r12;
        current->context.lr  = frame->lr;
    }

    tcb_t *next = &thread_table[current_thread_id];

    if (next->state == THREAD_STATE_TERMINATED) {
        next = &thread_table[0];
        current_thread_id = 0;
        next->state = THREAD_STATE_RUNNING;
    }

    frame->spsr = next->context.cpsr;
    frame->sp   = next->context.sp;
    frame->r0   = next->context.r0;
    frame->r1   = next->context.r1;
    frame->r2   = next->context.r2;
    frame->r3   = next->context.r3;
    frame->r4   = next->context.r4;
    frame->r5   = next->context.r5;
    frame->r6   = next->context.r6;
    frame->r7   = next->context.r7;
    frame->r8   = next->context.r8;
    frame->r9   = next->context.r9;
    frame->r10  = next->context.r10;
    frame->r11  = next->context.r11;
    frame->r12  = next->context.r12;
    frame->lr   = next->context.lr;
}

