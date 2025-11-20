#include <arch/cpu/interrupts.h>
#include <lib/kprintf.h>
#include <stdbool.h>
#include "lib/print_exception.h"  // Make sure the prototype is here

#include <arch/cpu/mode_registers.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>
// Helper for common handling
static void handle_exception(
    exc_frame_t* frame,
    const char* name,
    bool is_data_abort,
    bool is_prefetch_abort,
    unsigned int dfsr,
    unsigned int dfar,
    unsigned int ifsr,
    unsigned int ifar,
    unsigned int cpsr,
    unsigned int irq_spsr,
    unsigned int abort_spsr,
    unsigned int undefined_spsr,
    unsigned int supervisor_spsr
) {
    print_exception_infos(
        frame,
        name,
        0, // exception_source_addr (optional, can be frame->lr)
        is_data_abort,
        is_prefetch_abort,
        dfsr,
        dfar,
        ifsr,
        ifar,
        cpsr,
        irq_spsr,
        abort_spsr,
        undefined_spsr,
        supervisor_spsr
    );
}
#define UART_IRQ_BIT (1 << 25)

void software_interrupt_c(exc_frame_t* frame) {
    // For SVC, LR_svc holds the return address
    unsigned int svc_return_addr = frame->lr; // adjust if frame->lr is not the SVC LR
    // sometimes: svc_return_addr = get_svc_lr_from_stack(frame);

    print_exception_infos(
        frame,
        "Supervisor Call (SVC)",
        svc_return_addr,
        false, false, 0, 0, 0, 0, 0, 0, 0, 0, 0
    );

    uart_putc(4);  // End-of-transmission
    while(true) {} // Halt
}

void irq_c(exc_frame_t *frame) {
	if (gpu_interrupt->IRQPending2 & UART_IRQ_BIT) {
		uart_irq_handler();
	}

	(void)systimer_handle_irq();

	if (irq_debug) {
		handle_exception(frame, "IRQ", false, false, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	}
}

void fiq_c(exc_frame_t *frame) {

    handle_exception(frame, "FIQ", false, false, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    // End-of-transmission
    uart_putc(4);
    // Halt
    while (true) {}
}

void undefined_instruction_c(exc_frame_t *frame) {
    handle_exception(frame, "Undefined Instruction", false, false, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    // End-of-transmission
    uart_putc(4);
    // Halt
    while (true) {}
}

void prefetch_abort_c(exc_frame_t *frame) {
    unsigned int ifsr = read_ifsr();
    unsigned int ifar = read_ifar();
    handle_exception(frame, "Prefetch Abort", false, true, 0, 0, ifsr, ifar, 0, 0, 0, 0, 0);

    // End-of-transmission
    uart_putc(4);
    // Halt
    while (true) {}
}

void data_abort_c(exc_frame_t *frame) {
    unsigned int dfsr = read_dfsr();
    unsigned int dfar = read_dfar();

    unsigned int spsr_abt;
    asm volatile("mrs %0, SPSR" : "=r"(spsr_abt));

    handle_exception(frame,
                     "Data Abort",
                     true, false,
                     dfsr, dfar, 0, 0,  // DFSR/DFAR, IFSR/IFAR
                     0,                 // CPSR
                     0,                 // irq_spsr
                     spsr_abt,          // abort_spsr
                     0,                 // undefined_spsr
                     0);                // supervisor_spsr

    uart_putc(4);
    while(true) {}
}


void not_used_c(exc_frame_t *frame) {
    handle_exception(frame, "Not Used", false, false, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    // End-of-transmission
    uart_putc(4);
    // Halt
    while (true) {}
}

