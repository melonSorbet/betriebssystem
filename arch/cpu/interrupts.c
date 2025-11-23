#include <arch/cpu/interrupts.h>
#include <lib/kprintf.h>
#include <stdbool.h>
#include "lib/print_exception.h"  // Make sure the prototype is here

#include <arch/cpu/mode_registers.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>
// Helper for common handling

/*-------------------------------------------------------
  Helper: Handles all exceptions uniformly
-------------------------------------------------------*/
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
        frame->lr,
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

/*-------------------------------------------------------
  SWI / Supervisor Call
-------------------------------------------------------*/
void software_interrupt_c(exc_frame_t* frame) {
    unsigned int cpsr, spsr_svc;
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    spsr_svc = frame->spsr;  // SVC mode SPSR
    handle_exception(frame, "Supervisor Call",
                     false, false,
                     0, 0, 0, 0,
                     cpsr, 0, 0, 0, spsr_svc);
    uart_putc(4);
    while (true) {}
}

/*-------------------------------------------------------
  IRQ
-------------------------------------------------------*/
void irq_c(exc_frame_t *frame) {
    uint32_t pending1 = gpu_interrupt->IRQPending1;
    uint32_t pending2 = gpu_interrupt->IRQPending2;

    unsigned int cpsr, irq_spsr;
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    irq_spsr = frame->spsr; // SPSR_irq

    if (pending2 & UART_IRQ_BIT) {
        uart_irq_handler();
    }
    if (pending1 & SYSTIMER_IRQ_BIT) {
        systimer_handle_irq();
    }

    if (irq_debug) {
        handle_exception(frame, "IRQ",
                         false, false,
                         0, 0, 0, 0,
                         cpsr, irq_spsr, 0, 0, 0);
    }
}

/*-------------------------------------------------------
  FIQ
-------------------------------------------------------*/
void fiq_c(exc_frame_t *frame) {
    unsigned int cpsr;
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    handle_exception(frame, "FIQ",
                     false, false,
                     0, 0, 0, 0,
                     cpsr, 0, 0, 0, 0);
    uart_putc(4);
    while (true) {}
}

/*-------------------------------------------------------
  Undefined Instruction
-------------------------------------------------------*/
void undefined_instruction_c(exc_frame_t *frame) {
    unsigned int cpsr, und_spsr;
    und_spsr = frame->spsr; // SPSR_und
    asm volatile("mrs %0, cpsr" : "=r"(cpsr));
    handle_exception(frame, "Undefined Instruction",
                     false, false,
                     0, 0, 0, 0,
                     cpsr, 0, 0, und_spsr, 0);
    uart_putc(4);
    while (true) {}
}

/*-------------------------------------------------------
  Prefetch Abort
-------------------------------------------------------*/
void prefetch_abort_c(exc_frame_t *frame) {
    unsigned int ifsr = read_ifsr();
    unsigned int ifar = read_ifar();
    handle_exception(frame, "Prefetch Abort",
                     false, true,
                     0, 0, ifsr, ifar,
                     0, 0, 0, 0, 0);
    uart_putc(4);
    while (true) {}
}

/*-------------------------------------------------------
  Data Abort
-------------------------------------------------------*/
void data_abort_c(exc_frame_t *frame) {
    unsigned int dfsr = read_dfsr();
    unsigned int dfar = read_dfar();
    unsigned int spsr_abt;
    asm volatile("mrs %0, SPSR" : "=r"(spsr_abt)); // SPSR_abt

    handle_exception(frame, "Data Abort",
                     true, false,
                     dfsr, dfar, 0, 0,
                     0, 0, spsr_abt, 0, 0);
    uart_putc(4);
    while (true) {}
}

/*-------------------------------------------------------
  Not Used
-------------------------------------------------------*/
void not_used_c(exc_frame_t *frame) {
    handle_exception(frame, "Not Used",
                     false, false,
                     0, 0, 0, 0,
                     0, 0, 0, 0, 0);
    uart_putc(4);
    while (true) {}
}

