#include <arch/bsp/yellow_led.h>
#include <lib/kprintf.h>
#include <arch/bsp/uart.h>
#include "lib/kprintf.h"
#include <config.h>

#include "arch/bsp/uart.h"
#include <stdint.h>
#include <arch/bsp/systimer.h>
#include <tests/regcheck.h>

#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>
#include <arch/cpu/scheduler.h>
#include <lib/kprintf.h>
#include <config.h>
#include <user/main.h>
#include <arch/cpu/scheduler.h>
#include <stdarg.h>
void start_kernel [[noreturn]] (void);
void start_kernel [[noreturn]] (void)
{
    uart_init();
    systimer_init();
    scheduler_init();
    kprintf("=== Betriebssystem gestartet ===\n");
    test_kernel();
    scheduler_start();
}
