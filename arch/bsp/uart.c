#ifndef UART_INPUT_BUFFER_SIZE
#define UART_INPUT_BUFFER_SIZE 2048
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <lib/kprintf.h>
#include <lib/ringbuffer.h>
#include "arch/bsp/uart.h"

#define PL011_BUS_BASE	    0x7E201000
#define BUS_BASE	    0x7E000000
#define CPU_PERIPHERAL_BASE 0x3F000000
#define PL011_BASE	    ((PL011_BUS_BASE - BUS_BASE) + CPU_PERIPHERAL_BASE)
#define UART_IRQ_BIT	    (1 << 25)

typedef struct {
	volatile uint32_t DR;
	volatile uint32_t RSR_ECR;
	volatile uint32_t RESERVED0[4];
	volatile uint32_t FR;
	volatile uint32_t ILPR;
	volatile uint32_t RESERVED1;
	volatile uint32_t IBRD;
	volatile uint32_t FBRD;
	volatile uint32_t LCRH;
	volatile uint32_t CR;
	volatile uint32_t IFLS;
	volatile uint32_t IMSC;
	volatile uint32_t RIS;
	volatile uint32_t MIS;
	volatile uint32_t ICR;
	volatile uint32_t DMACR;
} UART;
bool					 irq_debug     = false;
volatile GPU_Interrupt_Controller *const gpu_interrupt = (GPU_Interrupt_Controller *)0x3F00B200;
static volatile UART *const		 uart	       = (UART *)(PL011_BASE);

#define PL011_FR_TXFF (1 << 5)
#define PL011_FR_RXFE (1 << 4)

#define PL011_LCRH_WLEN_8BIT (3 << 5)
#define PL011_LCRH_FEN	     (1 << 4)

#define PL011_CR_RXE	(1 << 9)
#define PL011_CR_TXE	(1 << 8)
#define PL011_CR_UARTEN (1 << 0)

#define PL011_INT_RXIM (1 << 4)
#define PL011_INT_RTIM (1 << 6)
#define PL011_INT_OEIM (1 << 1)
#define PL011_INT_RX   (1 << 4)
#define PL011_INT_RT   (1 << 6)
#define PL011_INT_OE   (1 << 1)

create_ringbuffer(uart_rx_buffer, UART_INPUT_BUFFER_SIZE);

void uart_init(void)
{
	uart->CR = 0;

	uart->ICR = 0x7FF;

	uart->LCRH = PL011_LCRH_WLEN_8BIT | PL011_LCRH_FEN;

	uart->IMSC = PL011_INT_RXIM | PL011_INT_RTIM | PL011_INT_OEIM;

	uart->CR = PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE;

	gpu_interrupt->EnableIRQs2 |= UART_IRQ_BIT;
}

void uart_putc(char input)
{
	while (uart->FR & PL011_FR_TXFF) {
	}
	uart->DR = (uint32_t)input;
}

void uart_puts(const char *string)
{
	while (*string) {
		uart_putc(*string++);
	}
}

char uart_getc(void)
{
	while (buff_is_empty(uart_rx_buffer)) {
	}
	return buff_getc(uart_rx_buffer);
}

int uart_getc_nonblock(void)
{
	if (buff_is_empty(uart_rx_buffer)) {
		return -1;
	}
	return (int)buff_getc(uart_rx_buffer);
}

bool uart_data_available(void)
{
	return !buff_is_empty(uart_rx_buffer);
}

void uart_irq_handler(void)
{
	if (uart->MIS & (PL011_INT_RX | PL011_INT_RT)) {
		while (!(uart->FR & PL011_FR_RXFE)) {
			uint32_t data = uart->DR;

			if (data & (1 << 8)) {
				volatile uint32_t error_clear = uart->RSR_ECR;
				(void)error_clear;
				continue;
			}

			char c = (char)(data & 0xFF);

			buff_putc(uart_rx_buffer, c);
		}
	}

	uart->ICR = PL011_INT_RX | PL011_INT_RT | PL011_INT_OE;
}

bool uart_tx_ready(void)
{
	return !(uart->FR & PL011_FR_TXFF);
}

bool uart_rx_ready(void)
{
	return !(uart->FR & PL011_FR_RXFE);
}

uint32_t uart_get_errors(void)
{
	return uart->RSR_ECR;
}
