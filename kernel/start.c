#include <arch/bsp/yellow_led.h>

static volatile unsigned int counter = 0;

static void increment_counter(void)
{
	counter++;
}

void start_kernel [[noreturn]] (void);
void start_kernel [[noreturn]] (void)
{
	init_yellow();
	yellow_on();

	// Endless counter
	while (true) {
		increment_counter();
	}
}
