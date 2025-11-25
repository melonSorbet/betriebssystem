#include <stdint.h>
#include <stdbool.h>
#include <lib/kprintf.h>
#include <config.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>

#define SYSTIMER_BUS_BASE 0x7E003000
#define BUS_BASE 0x7E000000
#define CPU_PERIPHERAL_BASE 0x3F000000
#define SYSTIMER_BASE (CPU_PERIPHERAL_BASE + (SYSTIMER_BUS_BASE - BUS_BASE))



typedef struct {
	volatile uint32_t CS;
	volatile uint32_t CLO;
	volatile uint32_t CHI;
	volatile uint32_t C0;
	volatile uint32_t C1;
	volatile uint32_t C2;
	volatile uint32_t C3;
} SystemTimer;
#define SYSTIMER_CS_M1   (1u << 1)


volatile SystemTimer *const systimer = (volatile SystemTimer *) SYSTIMER_BASE;

void systimer_init() {
	systimer->CS = SYSTIMER_CS_M1;

	uint32_t current_time = systimer->CLO;
	uint32_t next_time = current_time + TIMER_INTERVAL;
	systimer->C1 = next_time;
	gpu_interrupt->EnableIRQs1 |= SYSTIMER_IRQ_BIT;



}
bool systimer_handle_irq(void) {

    if ((systimer->CS & SYSTIMER_CS_M1) == 0) {
    	return false;

    }

     systimer->CS = SYSTIMER_CS_M1;

    kprintf("!\n");
    uint32_t current_time = systimer->CLO;
    uint32_t next_time = current_time + TIMER_INTERVAL;
    systimer->C1 = next_time;
    return true;
}
