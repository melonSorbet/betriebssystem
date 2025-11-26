#include <stdint.h>
#include <stdbool.h>
#include <lib/kprintf.h>
#include <config.h>
#include <arch/bsp/uart.h>
#include <arch/bsp/systimer.h>

#define SYSTIMER_BUS_BASE   0x7E003000
#define BUS_BASE	    0x7E000000
#define CPU_PERIPHERAL_BASE 0x3F000000
#define SYSTIMER_BASE	    (CPU_PERIPHERAL_BASE + (SYSTIMER_BUS_BASE - BUS_BASE))

static const unsigned int IRQ_CONTROLLER_BASE = 0x7E00B000 - 0x3F000000;

typedef struct {
	unsigned int CS;
	unsigned int CLO;
	unsigned int CHI;
	unsigned int unused1;
	unsigned int C1;
	unsigned int unused2;
	unsigned int C3;
} SystemTimer;
#define SYSTIMER_CS_M1	     (1u << 1)
#define TIMER_INTERRUPT_MASK (1 << 1)
#define UART_INTERRUPT_MASK  (1 << 25)
struct irq_controller {
	unsigned int unused[128]; // Registers 0x000-0x1FC (128 * 4 = 0x200)
	unsigned int IRQ_basic_pending;
	unsigned int PENDING_1;
	unsigned int PENDING_2;
	unsigned int FIQ_control;
	unsigned int ENABLE_1;
	unsigned int ENABLE_2;
};

static volatile struct irq_controller *const irq_controller_instance =
	(struct irq_controller *)IRQ_CONTROLLER_BASE;

#define TIMER_STATUS_1 (1 << 1) //C1 compare
#define TIMER_STATUS_3 (1 << 3) //C3 compare
volatile SystemTimer *const systimer = (volatile SystemTimer *)SYSTIMER_BASE;
void			    systimer_init(void)
{
	irq_controller_instance->ENABLE_1 |= TIMER_INTERRUPT_MASK;
	// Enable UART interrupt
	irq_controller_instance->ENABLE_2 |= UART_INTERRUPT_MASK;
	// Clear timer interrupt status bit
	systimer->CS |= TIMER_STATUS_1;
	unsigned int counter = systimer->CLO;
	systimer->C1	     = counter + TIMER_INTERVAL;
}
bool systimer_handle_irq(void)
{
	kprintf("!\n");
	unsigned int counter = systimer->CLO;
	unsigned int new_c1  = counter + TIMER_INTERVAL;

	systimer->C1 = new_c1;
	systimer->CS |= TIMER_STATUS_1;

	return false;
}
