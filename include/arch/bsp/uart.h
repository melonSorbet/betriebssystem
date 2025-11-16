
#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void uart_loopback(void);
char uart_getc(void);
void uart_putc(char input);
void uart_puts(const char *string);
void uart_irq_handler(void);

typedef struct {
    volatile uint32_t IRQBasicPending;
    volatile uint32_t IRQPending1;
    volatile uint32_t IRQPending2;
    volatile uint32_t FIQControl;
    volatile uint32_t EnableIRQs1;
    volatile uint32_t EnableIRQs2;
    volatile uint32_t EnableBasicIRQs;
    volatile uint32_t DisableIRQs1;
    volatile uint32_t DisableIRQs2;
    volatile uint32_t DisableBasicIRQs;
} GPU_Interrupt_Controller;

extern volatile GPU_Interrupt_Controller* const gpu_interrupt;
extern bool irq_debug;

#define UART_IRQ_BIT (1 << 25)

#endif // UART_H

