
#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void uart_init(void);
void uart_loopback(void);
char uart_getc(void);
void uart_putc(char input);
void uart_puts(const char *string);
void uart_irq_handler(void);


enum gpio_func {
	gpio_input  = 0x0,
	gpio_output = 0x1,
};

struct gpio {
	unsigned int func[6];
	unsigned int unused0;
	unsigned int set[2];
	unsigned int unused1;
	unsigned int clr[2];
};

static const unsigned int GPIO_BASE		   = 0x7E200000 - 0x3F000000;
static const unsigned int YELLOW_LED	   = 7u;
static const unsigned int GPF_BITS		   = 3u;
static const unsigned int GPF_MASK		   = 0x7u;
static const unsigned int YELLOW_LED_GPF_SHIFT = YELLOW_LED * GPF_BITS;

static volatile struct gpio *const gpio_port = (struct gpio *)GPIO_BASE;



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

