#include <arch/bsp/yellow_led.h>
#include <lib/kprintf.h>
#include <arch/bsp/uart.h>
#include "lib/kprintf.h"
#include <config.h>
static volatile unsigned int counter = 0;

/*static void increment_counter(void)
{
	counter++;
}
*/

#include "arch/bsp/uart.h"
#include <stdint.h>

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
void start_kernel [[noreturn]] (void);
void start_kernel [[noreturn]] (void)
{
	test_kprintf();
	kprintf("=== Betriebssystem gestartet ===\n");
	test_kernel();

	while (true) {
		char c = uart_getc();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-to-pointer-cast"
		kprintf("Es wurde folgendes Zeichen eingegeben: %c, In Hexadezimal: %x, "
			"In Dezimal: %08i, Als Ptr: %p\n",
			c, (unsigned int)c, (int)c, (void *)c);
#pragma GCC diagnostic pop
	}
}
