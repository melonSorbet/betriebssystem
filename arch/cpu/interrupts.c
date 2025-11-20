#include <arch/cpu/interrupts.h>
#include <lib/kprintf.h>
#include <stdbool.h>
#include "lib/print_exception.h" // Make sure the prototype is here

#include <arch/cpu/mode_registers.h>
#include <arch/bsp/uart.h>
// Helper for common handling
static void handle_exception(exc_frame_t *frame, const char *name, bool is_data_abort,
			     bool is_prefetch_abort, unsigned int dfsr, unsigned int dfar,
			     unsigned int ifsr, unsigned int ifar, unsigned int cpsr,
			     unsigned int irq_spsr, unsigned int abort_spsr,
			     unsigned int undefined_spsr, unsigned int supervisor_spsr)
{
	print_exception_infos(frame, name,
			      0, // exception_source_addr (optional, can be frame->lr)
			      is_data_abort, is_prefetch_abort, dfsr, dfar, ifsr, ifar, cpsr,
			      irq_spsr, abort_spsr, undefined_spsr, supervisor_spsr);
}
#define UART_IRQ_BIT (1 << 25)

void software_interrupt_c(exc_frame_t *frame)
{
	unsigned int svc_return_addr = frame->lr - 4; // LR_svc points after SVC
	print_exception_infos(frame, "Supervisor Call (SVC)", svc_return_addr, false, false, 0, 0,
			      0, 0, 0, 0, 0, 0, 0);
	uart_putc(4); // End-of-transmission
	while (true) {
	}
}

void irq_c(exc_frame_t *frame)
{
	uint32_t lr;
	asm volatile("mov %0, lr" : "=r"(lr));
	uint32_t source = lr - 4;

	if (gpu_interrupt->IRQPending2 & UART_IRQ_BIT) {
		uart_irq_handler();
	}

	handle_exception(frame, "IRQ", false, false, 0, 0, 0, 0, source, 0, 0, 0, 0);
}

void fiq_c(exc_frame_t *frame)
{
	uint32_t lr;
	asm volatile("mov %0, lr" : "=r"(lr));
	uint32_t source = lr - 4;

	handle_exception(frame, "FIQ", false, false, 0, 0, 0, 0, source, 0, 0, 0, 0);

	uart_putc(4);
	while (true) {
	}
}

void undefined_instruction_c(exc_frame_t *frame)
{
	uint32_t lr, spsr_und;
	asm volatile("mov %0, lr" : "=r"(lr));
	asm volatile("mrs %0, SPSR" : "=r"(spsr_und));
	uint32_t origin = lr - 4;

	print_exception_infos(frame, "Undefined Instruction", origin, false, false, 0, 0, 0, 0, 0,
			      0, 0, spsr_und, 0);

	uart_putc(4);
	while (1) {
	}
}

void prefetch_abort_c(exc_frame_t *frame)
{
	unsigned int ifsr = read_ifsr(); // Instruction Fault Status Register
	unsigned int ifar = read_ifar(); // Instruction Fault Address Register

	// Read current CPSR
	uint32_t cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));

	// Read abort mode SPSR for printing
	uint32_t spsr_abt;
	asm volatile("mrs %0, SPSR_abt" : "=r"(spsr_abt)); // if your assembler supports banked SPSR

	print_exception_infos(frame, "Prefetch Abort",
			      ifar, // Exception source = IFAR
			      false, true, 0, 0, // DFSR / DFAR
			      ifsr, ifar, // IFSR / IFAR
			      cpsr, 0, 0, spsr_abt, 0); // Pass SPSR for abort

	uart_putc(4); // EOT
	while (1) {
	} // Halt CPU
}

void data_abort_c(exc_frame_t *frame)
{
	uint32_t lr_abt;
	asm volatile("mov %0, lr" : "=r"(lr_abt)); // LR in abort mode
	uint32_t source_addr = lr_abt - 8; // Data Abort: LR points after faulting instr

	unsigned int dfsr = read_dfsr(); // Data Fault Status Register
	unsigned int dfar = read_dfar(); // Data Fault Address Register

	// Read banked abort mode SPSR
	uint32_t spsr_abt;
	asm volatile("mrs %0, SPSR" : "=r"(spsr_abt)); // Banked SPSR

	print_exception_infos(frame, "Data Abort", source_addr, true, false, dfsr, dfar, 0,
			      0, // IFSR / IFAR
			      0, // CPSR (can leave 0)
			      0, // IRQ SPSR
			      spsr_abt, // Abort SPSR
			      0, // Undefined SPSR
			      0); // Supervisor SPSR

	uart_putc(4);
	while (1) {
	}
}

void not_used_c(exc_frame_t *frame)
{
	uint32_t lr;
	asm volatile("mov %0, lr" : "=r"(lr));
	uint32_t source = lr - 4;

	handle_exception(frame, "Not Used", false, false, 0, 0, 0, 0, source, 0, 0, 0, 0);

	uart_putc(4);
	while (true) {
	}
}
