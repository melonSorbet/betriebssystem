#include <arch/bsp/yellow_led.h>
#include <lib/kprintf.h>
#include <arch/bsp/uart.h>
#include "lib/kprintf.h"
#include <config.h>

#include "arch/bsp/uart.h"
#include <stdint.h>
#include <arch/bsp/systimer.h>
#include <tests/regcheck.h>
void test_kprintf(void)
{
	int	     my_var_i	= -12345;
	unsigned int my_var_u	= 123;
	unsigned int my_var_hex = 0xABCD;
	void	    *ptr	= (void *)0xDEADBEEF;

	kprintf("=== kprintf Test Start ===\n");

	// Basic specifiers
	kprintf("Char: %c\n", 'A'); // %c
	kprintf("String: %s\n", "Hello World"); // %s
	kprintf("Signed int: %i\n", my_var_i); // %i
	kprintf("Unsigned int: %u\n", my_var_u); // %u
	kprintf("Hex: %x\n", my_var_hex); // %x
	kprintf("Pointer: %p\n", ptr); // %p
	kprintf("Percent sign: %%\n"); // %%

	// Width 8 (space-padded)
	kprintf("Signed int width 8: %8i\n", my_var_i); // space padding
	kprintf("Unsigned int width 8: %8u\n", my_var_u); // space padding
	kprintf("Hex width 8: %8x\n", my_var_hex); // space padding

	// Width 08 (zero-padded)
	kprintf("Signed int zero-padded: %08i\n", my_var_i); // zero padding
	kprintf("Unsigned int zero-padded: %08u\n", my_var_u); // zero padding
	kprintf("Hex zero-padded: %08x\n", my_var_hex); // zero padding

	// Pointer already prints as 0xXXXXXXXX
	kprintf("Pointer test: %p\n", ptr);

	kprintf("=== kprintf Test End ===\n");
}
void do_data_abort(void)
{
	volatile unsigned int *ptr = (volatile unsigned int *)0x1;
	*ptr			   = 0xDEADBEEF;
}

void do_prefetch_abort(void)
{
	void (*bad_func)(void) = (void (*)(void))0xDEADBEEF;
	bad_func(); // Prefetch Abort
}

void do_supervisor_call(void)
{
	asm volatile("svc 0");
}

void do_undefined_inst(void)
{
	asm volatile(".word 0xE7F000F0"); // undefined instruction
}

static void subprogram [[noreturn]] (void)
{
	while (true) {
		char c = uart_getc();
		for (unsigned int n = 0; n < PRINT_COUNT; n++) {
			uart_putc(c);
			volatile unsigned int i = 0;
			for (; i < BUSY_WAIT_COUNTER; i++) {
			}
		}
	}
}
#include <arch/cpu/scheduler.h>
void start_kernel [[noreturn]] (void);
void start_kernel [[noreturn]] (void)
{
	uart_init();
	systimer_init();
	scheduler_init();
	scheduler_start();
	kprintf("=== Betriebssystem gestartet ===\n");
	test_kernel();
	while (true) {
		char c = uart_getc();

		switch (c) {
		case 'd':
			kprintf("debug mode activated");
			irq_debug = !irq_debug;
			break;
		case 'a':
			do_data_abort();
			break;
		case 'p':
			do_prefetch_abort();
			break;
		case 's':
			do_supervisor_call();
			break;
		case 'u':
			do_undefined_inst();
			break;
		case 'c':
			register_checker();
			break;
		case 'e':
			subprogram();
		default:
			kprintf("Unknown input: %c\n", c);
			break;
		}
	}
}
