#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>
#include <arch/cpu/interrupts.h>
#define MAX_THREADS	  32
#define THREAD_STACK_SIZE 1024
#define IDLE_THREAD_ID	  0

typedef enum {
	THREAD_STATE_READY,
	THREAD_STATE_RUNNING,
	THREAD_STATE_TERMINATED
} thread_state_t;

typedef struct {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t sp;
	uint32_t lr;
	uint32_t pc;
	uint32_t cpsr;
} thread_context_t;

typedef struct {
	thread_context_t context;
	thread_state_t	 state;
	uint32_t	 thread_id;
	uint8_t		 stack[THREAD_STACK_SIZE];
} tcb_t;
void   scheduler_init(void);
void   scheduler_start [[noreturn]] (void);
void   scheduler_thread_create(void (*func)(void *), const void *arg, unsigned int arg_size);
void   scheduler_schedule(void);
void   scheduler_terminate_current_thread(void);
tcb_t *scheduler_get_current_thread(void);
void   syscall_exit(void);
void   scheduler_context_switch(exc_frame_t *frame);
#endif
