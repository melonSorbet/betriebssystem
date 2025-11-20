#include <arch/cpu/interrupts.h>
#include <lib/kprintf.h>
#include <stdbool.h>
#include "lib/print_exception.h"  // Make sure the prototype is here

#include <arch/cpu/mode_registers.h>
#include <arch/bsp/uart.h>
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
    unsigned int svc_return_addr = frame->lr - 4; // LR in frame already points after SVC
    uint32_t spsr_svc = frame->spsr;

    print_exception_infos(
        frame,
        "Supervisor Call (SVC)",
        svc_return_addr,
        false, false,
        0, 0, 0, 0,
        0,      // CPSR
        0,      // IRQ SPSR
        0,      // Abort SPSR
        0,      // Undefined SPSR
        spsr_svc
    );

    uart_putc(4);
    while(1) {}
}

void irq_c(exc_frame_t *frame) {
    uint32_t spsr_irq = frame->spsr;  // read SPSR saved by stub
    unsigned int source_addr = frame->lr - 4; // LR in frame

    if (gpu_interrupt->IRQPending2 & UART_IRQ_BIT) {
        uart_irq_handler();
    }

    handle_exception(frame, "IRQ", false, false,
                     0, 0, 0, 0,
                     source_addr,
                     spsr_irq, 0, 0, 0);
}

void fiq_c(exc_frame_t *frame) {
    uint32_t spsr_fiq = frame->spsr;
    unsigned int source_addr = frame->lr - 4;

    handle_exception(frame, "FIQ", false, false,
                     0, 0, 0, 0,
                     source_addr,
                     0, 0, spsr_fiq, 0);

    uart_putc(4);
    while(1) {}
}



void undefined_instruction_c(exc_frame_t *frame) {
    uint32_t spsr_und = frame->spsr;
    unsigned int origin = frame->lr - 4;

    print_exception_infos(frame,
                          "Undefined Instruction",
                          origin,
                          false, false,
                          0, 0, 0, 0,
                          0, 0, 0, spsr_und, 0);

    uart_putc(4);
    while(1) {}
}





void prefetch_abort_c(exc_frame_t *frame) {
    unsigned int ifsr = read_ifsr();
    unsigned int ifar = read_ifar();

    uint32_t spsr_abt = frame->spsr; // correct now

    print_exception_infos(frame,
                          "Prefetch Abort",
                          ifar,
                          false, true,
                          0, 0,
                          ifsr, ifar,
                          0,     // CPSR optional
                          0,     // IRQ SPSR
                          0,     // Abort SPSR
                          spsr_abt, // Undefined SPSR slot used
                          0);

    uart_putc(4);
    while(1) {}
}



void data_abort_c(exc_frame_t *frame) {
    unsigned int dfsr = read_dfsr();
    unsigned int dfar = read_dfar();

    uint32_t spsr_abt = frame->spsr;

    print_exception_infos(frame,
                          "Data Abort",
                          frame->lr - 8,
                          true, false,
                          dfsr, dfar,
                          0, 0,
                          0,
                          0,       // IRQ SPSR
                          spsr_abt,// Abort SPSR
                          0,
                          0);

    uart_putc(4);
    while(1) {}
}


void not_used_c(exc_frame_t *frame) {
    unsigned int source_addr = frame->lr - 4; // use frame LR
    uint32_t spsr_unused = frame->spsr;      // use saved SPSR

    handle_exception(frame,
                     "Not Used",
                     false, false,
                     0, 0, 0, 0,
                     source_addr,
                     0,       // irq_spsr
                     0,       // abort_spsr
                     spsr_unused, // undefined_spsr (or any slot, since it's not really used)
                     0);

    uart_putc(4);
    while(true) {}
}
