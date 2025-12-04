#include <config.h>
#include <arch/bsp/uart.h>
#include <tests/regcheck.h>
#include <user/main.h>

void do_data_abort(void)
{
	volatile unsigned int *ptr = (volatile unsigned int *)0x1;
	*ptr = 0xDEADBEEF;
}

void do_prefetch_abort(void)
{
	void (*bad_func)(void) = (void (*)(void))0xDEADBEEF;
	bad_func();
}

void do_svc(void)
{
	asm volatile("svc 0");
}

void do_undef(void)
{
	asm volatile(".word 0xE7F000F0");
}

void main(void *args) {
	test_user(args);
	char c = *((char *)args);
	
	switch (c) {
		case 'a':
			do_data_abort();
			return;
		case 'p':
			do_prefetch_abort();
			return;
		case 'u':
			do_undef();
			return;
		case 's':
			do_svc();
			return;
		case 'c':
			register_checker();
			return;
	}
	
	for(unsigned int n = 0; n < PRINT_COUNT; n++){
		for(volatile unsigned int i = 0; i < BUSY_WAIT_COUNTER; i++){}
		uart_putc(c);
	}
}
