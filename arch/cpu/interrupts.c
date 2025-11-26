#include <arch/cpu/interrupts.h>
#include <lib/kprintf.h>
#include <stdbool.h>
#include "lib/print_exception.h" // Make sure the prototype is here

#include <arch/cpu/mode_registers.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>

// ARM Processor Mode definitions
#define PSR_MODE_MASK 0x1F
#define PSR_USR	      0x10 // User mode
#define PSR_FIQ	      0x11 // FIQ mode
#define PSR_IRQ	      0x12 // IRQ mode
#define PSR_SVC	      0x13 // Supervisor mode
#define PSR_ABT	      0x17 // Abort mode
#define PSR_UND	      0x1B // Undefined mode
#define PSR_SYS	      0x1F // System mode

/*-------------------------------------------------------
  Read all mode-specific SPSRs for exception handling
-------------------------------------------------------*/
static void read_all_spsrs(exc_frame_t *frame, unsigned int *irq_spsr, unsigned int *abort_spsr,
			   unsigned int *undefined_spsr, unsigned int *supervisor_spsr)
{
	unsigned int original_cpsr;
	unsigned int current_mode;

	asm volatile("mrs %0, cpsr" : "=r"(original_cpsr));
	current_mode = original_cpsr & PSR_MODE_MASK;

	// Disable interrupts while mode switching
	asm volatile("cpsid if");

	// Read IRQ mode SPSR
	asm volatile("cps %0" : : "i"(PSR_IRQ));
	asm volatile("mrs %0, spsr" : "=r"(*irq_spsr));

	// Read Abort mode SPSR
	asm volatile("cps %0" : : "i"(PSR_ABT));
	asm volatile("mrs %0, spsr" : "=r"(*abort_spsr));

	// Read Undefined mode SPSR
	asm volatile("cps %0" : : "i"(PSR_UND));
	asm volatile("mrs %0, spsr" : "=r"(*undefined_spsr));

	// Read Supervisor mode SPSR
	asm volatile("cps %0" : : "i"(PSR_SVC));
	asm volatile("mrs %0, spsr" : "=r"(*supervisor_spsr));

	// Restore original mode and interrupt state
	asm volatile("msr cpsr_cxsf, %0" : : "r"(original_cpsr));

	// Override with the actual SPSR from the current exception frame
	// This ensures we get the correct SPSR for the mode that triggered the exception
	if (frame && frame->spsr) {
		switch (current_mode) {
		case PSR_SVC:
			*supervisor_spsr = frame->spsr;
			break;
		case PSR_IRQ:
			*irq_spsr = frame->spsr;
			break;
		case PSR_ABT:
			*abort_spsr = frame->spsr;
			break;
		case PSR_UND:
			*undefined_spsr = frame->spsr;
			break;
		}
	}
}

/*-------------------------------------------------------
  Helper: Handles all exceptions uniformly
-------------------------------------------------------*/
static void handle_exception(exc_frame_t *frame, const char *name, bool is_data_abort,
			     bool is_prefetch_abort, unsigned int dfsr, unsigned int dfar,
			     unsigned int ifsr, unsigned int ifar, unsigned int cpsr)
{
	unsigned int irq_spsr	     = 0;
	unsigned int abort_spsr	     = 0;
	unsigned int undefined_spsr  = 0;
	unsigned int supervisor_spsr = 0;

	// Read all SPSRs from different modes
	read_all_spsrs(frame, &irq_spsr, &abort_spsr, &undefined_spsr, &supervisor_spsr);

	print_exception_infos(frame, name, frame->lr, is_data_abort, is_prefetch_abort, dfsr, dfar,
			      ifsr, ifar, cpsr, irq_spsr, abort_spsr, undefined_spsr,
			      supervisor_spsr);
}

/*-------------------------------------------------------
  SWI / Supervisor Call
-------------------------------------------------------*/
void software_interrupt_c(exc_frame_t *frame)
{
	unsigned int cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));
	handle_exception(frame, "Supervisor Call", false, false, 0, 0, 0, 0, cpsr);
	uart_putc(4);
	while (true) {
	}
}

/*-------------------------------------------------------
  IRQ
-------------------------------------------------------*/
void irq_c(exc_frame_t *frame)
{
	uint32_t pending1 = gpu_interrupt->IRQPending1;
	uint32_t pending2 = gpu_interrupt->IRQPending2;

	unsigned int cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));

	if (pending2 & UART_IRQ_BIT) {
		uart_irq_handler();
	}
	if (pending1 & SYSTIMER_IRQ_BIT) {
		systimer_handle_irq();
	}

	if (irq_debug) {
		handle_exception(frame, "IRQ", false, false, 0, 0, 0, 0, cpsr);
	}
}

/*-------------------------------------------------------
  FIQ
-------------------------------------------------------*/
void fiq_c(exc_frame_t *frame)
{
	unsigned int cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));
	handle_exception(frame, "FIQ", false, false, 0, 0, 0, 0, cpsr);
	uart_putc(4);
	while (true) {
	}
}

/*-------------------------------------------------------
  Undefined Instruction
-------------------------------------------------------*/
void undefined_instruction_c(exc_frame_t *frame)
{
	unsigned int cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));
	handle_exception(frame, "Undefined Instruction", false, false, 0, 0, 0, 0, cpsr);
	uart_putc(4);
	while (true) {
	}
}

/*-------------------------------------------------------
  Prefetch Abort
-------------------------------------------------------*/
void prefetch_abort_c(exc_frame_t *frame)
{
	unsigned int ifsr = read_ifsr();
	unsigned int ifar = read_ifar();
	unsigned int cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));

	handle_exception(frame, "Prefetch Abort", false, true, 0, 0, ifsr, ifar, cpsr);
	uart_putc(4);
	while (true) {
	}
}

/*-------------------------------------------------------
  Data Abort
-------------------------------------------------------*/
void data_abort_c(exc_frame_t *frame)
{
	unsigned int dfsr = read_dfsr();
	unsigned int dfar = read_dfar();
	unsigned int cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));

	handle_exception(frame, "Data Abort", true, false, dfsr, dfar, 0, 0, cpsr);
	uart_putc(4);
	while (true) {
	}
}

/*-------------------------------------------------------
  Not Used
-------------------------------------------------------*/
void not_used_c(exc_frame_t *frame)
{
	unsigned int cpsr;
	asm volatile("mrs %0, cpsr" : "=r"(cpsr));

	handle_exception(frame, "Not Used", false, false, 0, 0, 0, 0, cpsr);
	uart_putc(4);
	while (true) {
	}
}
