#include <stdint.h>
void software_interrupt_c(uint32_t *frame) { }
void irq_c(uint32_t *frame) { }
void fiq_c(uint32_t *frame) { }
void undefined_instruction_c(uint32_t *frame) { }
void prefetch_abort_c(uint32_t *frame) {}
void data_abort_c(uint32_t *frame) { }
void not_used_c(uint32_t *frame) { }
