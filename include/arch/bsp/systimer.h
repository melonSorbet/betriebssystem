#ifndef ARCH_BSP_SYSTIMER_H
#define ARCH_BSP_SYSTIMER_H

#include <stdint.h>
#include <stdbool.h>

void systimer_init(void);
bool systimer_handle_irq(void);
#endif