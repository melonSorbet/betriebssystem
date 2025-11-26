#ifndef ARCH_BSP_SYSTIMER_H
#define ARCH_BSP_SYSTIMER_H

#include <stdint.h>
#include <stdbool.h>

#define SYSTIMER_IRQ_BIT (1u << 1)

void systimer_init(void);
bool systimer_handle_irq(void);
#endif

