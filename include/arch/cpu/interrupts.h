#ifndef EXC_FRAME_H
#define EXC_FRAME_H

#include <stdint.h>

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
	uint32_t lr;
	uint32_t spsr; // <-- save SPSR here
} exc_frame_t;

/* Exception C handlers */
void software_interrupt_c(exc_frame_t *frame);
void irq_c(exc_frame_t *frame);
void fiq_c(exc_frame_t *frame);
void undefined_instruction_c(exc_frame_t *frame);
void prefetch_abort_c(exc_frame_t *frame);
void data_abort_c(exc_frame_t *frame);
void not_used_c(exc_frame_t *frame);

#endif
