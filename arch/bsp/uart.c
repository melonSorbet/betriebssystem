#ifndef UART_INPUT_BUFFER_SIZE
#define UART_INPUT_BUFFER_SIZE 256
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

// GPIO
#define GPIO_BASE (CPU_PERIPHERAL_BASE + 0x200000)
#define GPFSEL1	  ((volatile uint32_t *)(GPIO_BASE + 0x04))
#define GPPUD	  ((volatile uint32_t *)(GPIO_BASE + 0x94))
#define GPPUDCLK0 ((volatile uint32_t *)(GPIO_BASE + 0x98))

#define TXD_PIN	       14
#define RXD_PIN	       15
#define TXD_PIN_INDEX  (TXD_PIN % 10)
#define RXD_PIN_INDEX  (RXD_PIN % 10)
#define GPIO_FUNC_BITS 3
#define TXD_SHIFT      (TXD_PIN_INDEX * GPIO_FUNC_BITS)
#define RXD_SHIFT      (RXD_PIN_INDEX * GPIO_FUNC_BITS)
#define GPIO_FUNC_MASK 0x7
#define GPIO_ALT0      0x4

// UART registers
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

bool irq_debug = false;

// Use the GPU_Interrupt_Controller definition from uart.h
volatile GPU_Interrupt_Controller *const gpu_interrupt = (GPU_Interrupt_Controller *)0x3F00B200;

static volatile UART *const uart = (UART *)(PL011_BASE);

#define UART_IRQ_BIT (1 << 25)

// Flags
#define PL011_FR_TXFF (1 << 5) // Transmit FIFO full
#define PL011_FR_RXFE (1 << 4) // Receive FIFO empty
#define PL011_FR_BUSY (1 << 3) // UART busy

// Line control
#define PL011_LCRH_WLEN_8BIT (3 << 5)
#define PL011_LCRH_FEN	     (1 << 4) // FIFO enable
#define PL011_LCRH_STP2	     (1 << 3) // Two stop bits

// Control
#define PL011_CR_RXE	(1 << 9) // Receive enable
#define PL011_CR_TXE	(1 << 8) // Transmit enable
#define PL011_CR_UARTEN (1 << 0) // UART enable

// Interrupt bits
#define PL011_INT_RXIM (1 << 4) // Receive interrupt mask
#define PL011_INT_TXIM (1 << 5) // Transmit interrupt mask
#define PL011_INT_RTIM (1 << 6) // Receive timeout interrupt mask
#define PL011_INT_OEIM (1 << 1) // Overrun error interrupt mask

// Interrupt status bits (RIS/MIS)
#define PL011_INT_RX (1 << 4) // Receive interrupt
#define PL011_INT_TX (1 << 5) // Transmit interrupt
#define PL011_INT_RT (1 << 6) // Receive timeout interrupt
#define PL011_INT_OE (1 << 1) // Overrun error interrupt

// =============================================================
//                      RX RING BUFFER
// =============================================================
create_ringbuffer(uart_rx_buffer, UART_INPUT_BUFFER_SIZE);

// =============================================================
//                       UART INIT
// =============================================================
void uart_init(void)
{
	// ---------------- GPIO Configuration ----------------
	// Disable pull-up/down for TXD and RXD pins
	*GPPUD = 0x00; // No pull-up/down
	asm volatile("dsb");

	// Wait for 150 cycles
	for (volatile int i = 0; i < 150; i++) {
		asm volatile("nop");
	}

	// Clock the control signals for TXD and RXD
	*GPPUDCLK0 = (1 << TXD_PIN) | (1 << RXD_PIN);
	asm volatile("dsb");

	// Wait for 150 cycles
	for (volatile int i = 0; i < 150; i++) {
		asm volatile("nop");
	}

	*GPPUDCLK0 = 0; // Remove clock
	asm volatile("dsb");

	// Set GPIO pins to ALT0 function
	uint32_t gpfsel1_val = *GPFSEL1;
	gpfsel1_val &= ~(GPIO_FUNC_MASK << TXD_SHIFT);
	gpfsel1_val &= ~(GPIO_FUNC_MASK << RXD_SHIFT);
	gpfsel1_val |= (GPIO_ALT0 << TXD_SHIFT);
	gpfsel1_val |= (GPIO_ALT0 << RXD_SHIFT);
	*GPFSEL1 = gpfsel1_val;

	// ---------------- Disable UART ----------------
	uart->CR = 0;

	// Clear all pending interrupts
	uart->ICR = 0x7FF;

	// ---------------- Baud rate configuration ----------------
	// For 3MHz clock and 115200 baud:
	// IBRD = 3,000,000 / (16 * 115200) = 1.627 -> 1
	// FBRD = integer((0.627 * 64) + 0.5) = 40
	uart->IBRD = 1; // Integer baud rate divisor
	uart->FBRD = 40; // Fractional baud rate divisor

	// ---------------- 8-bit + FIFO -----------------
	uart->LCRH = PL011_LCRH_WLEN_8BIT | PL011_LCRH_FEN;

	// ---------------- Enable RX interrupts ---------
	uart->IMSC = PL011_INT_RXIM | PL011_INT_RTIM | PL011_INT_OEIM;

	// ---------------- Enable UART ------------------
	uart->CR = PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE;

	// ---------------- Enable IRQ in GPU ------------
	gpu_interrupt->EnableIRQs2 |= UART_IRQ_BIT;

	asm volatile("dsb");
}

// =============================================================
//                  UART TX ROUTINES
// =============================================================
void uart_putc(char input)
{
	// Wait until transmit FIFO is not full
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

// =============================================================
//                  UART RX ROUTINES
// =============================================================

// --- Fully interrupt-driven, blocking ---
char uart_getc(void)
{
	// According to BCM2835 docs, we should check for errors
	// when reading from the data register
	while (buff_is_empty(uart_rx_buffer)) {
	}
	return buff_getc(uart_rx_buffer);
}

// --- Non-blocking, returns -1 if empty ---
int uart_getc_nonblock(void)
{
	if (buff_is_empty(uart_rx_buffer)) {
		return -1;
	}
	return (int)buff_getc(uart_rx_buffer);
}

// --- Check if data is available ---
bool uart_data_available(void)
{
	return !buff_is_empty(uart_rx_buffer);
}

// =============================================================
//                       IRQ HANDLER
// =============================================================
void uart_irq_handler(void)
{
	// Read the masked interrupt status to see which interrupts are active
	uint32_t mis = uart->MIS;

	// Handle receive interrupt
	if (mis & (PL011_INT_RX | PL011_INT_RT)) {
		// Drain RX FIFO until empty
		while (!(uart->FR & PL011_FR_RXFE)) {
			uint32_t data = uart->DR;

			// Check for error flags (bits 8-11 in DR)
			if (data & (1 << 8)) {
				// Framing error, break error, parity error, or overrun error
				// Clear error by reading RSR/ECR (we don't use this register)
				volatile uint32_t error_clear = uart->RSR_ECR;
				(void)error_clear; // Suppress unused variable warning
				continue; // Skip this corrupted character
			}

			// Extract the actual character (bits 0-7)
			char c = (char)(data & 0xFF);

			// Put character in ring buffer (drops if full)
			buff_putc(uart_rx_buffer, c);
		}
	}

	// Handle overrun error
	if (mis & PL011_INT_OE) {
		// Overrun error occurred - clear by reading data register
		// We don't need to do anything special as we're already draining FIFO
	}

	// Clear the specific interrupts we handled
	// According to PL011 docs, we should only clear the interrupts we handled
	uart->ICR = mis & (PL011_INT_RX | PL011_INT_RT | PL011_INT_OE);

	// Memory barriers to ensure completion
	asm volatile("dsb");
	asm volatile("isb");
}

// =============================================================
//                  UART STATUS FUNCTIONS
// =============================================================

// Check if UART is ready to transmit
bool uart_tx_ready(void)
{
	return !(uart->FR & PL011_FR_TXFF);
}

// Check if UART has received data in hardware FIFO
bool uart_rx_ready(void)
{
	return !(uart->FR & PL011_FR_RXFE);
}

// Get UART error status
uint32_t uart_get_errors(void)
{
	return uart->RSR_ECR;
}
