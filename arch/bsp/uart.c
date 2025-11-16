
#ifndef UART_INPUT_BUFFER_SIZE
#define UART_INPUT_BUFFER_SIZE 256
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <lib/kprintf.h>
#include "arch/bsp/uart.h"

#define PL011_BUS_BASE 0x7E201000
#define BUS_BASE 0x7E000000
#define CPU_PERIPHERAL_BASE 0x3F000000
#define PL011_BASE ((PL011_BUS_BASE - BUS_BASE) + CPU_PERIPHERAL_BASE)

typedef struct {
    volatile uint32_t DR;
    volatile uint32_t RSR_ECR;
    volatile uint32_t RESERVED0[4];
    volatile uint32_t FR;
    volatile uint32_t ILPR;
    volatile uint32_t RESERVED2;
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
volatile GPU_Interrupt_Controller* const gpu_interrupt =
    (GPU_Interrupt_Controller*)0x3F00B200; // Update with correct CPU physical address
bool irq_debug = false;
static volatile UART* const uart = (UART*)(PL011_BASE);

#define UART_IRQ_BIT (1 << 25)
// --- Flags ---
#define PL011_FR_TXFF   (1 << 5)
#define PL011_FR_RXFE   (1 << 4)

// --- Line Control ---
#define PL011_LCRH_WLEN_8BIT (3 << 5)
#define PL011_LCRH_FEN  (1 << 4)

// --- Control ---
#define PL011_CR_RXE    (1 << 9)
#define PL011_CR_TXE    (1 << 8)
#define PL011_CR_UARTEN (1 << 0)

// --- Interrupts ---
#define PL011_INT_RXIM  (1 << 4)
#define PL011_INT_RTIM  (1 << 6)
#define PL011_INT_OEIM  (1 << 1)

// --- ARM Local Interrupt Controller ---
#define LOCAL_PERIPHERAL_BASE 0x40000000
#define CORE0_IRQ_SOURCE ((volatile uint32_t*)(LOCAL_PERIPHERAL_BASE + 0x60))

// --- Ring buffer ---
static volatile char uart_rx_ring_buffer[UART_INPUT_BUFFER_SIZE];
static volatile unsigned int uart_rx_head = 0;
static volatile unsigned int uart_rx_tail = 0;

// --- Initialization ---
void uart_init(void) {
    // Disable UART
    uart->CR = 0x0;

    // Clear interrupts
    uart->ICR = 0x7FF;

    // Enable FIFOs and 8-bit word length
    uart->LCRH = PL011_LCRH_WLEN_8BIT | PL011_LCRH_FEN;

    // Enable interrupts
    uart->IMSC = PL011_INT_RXIM | PL011_INT_RTIM | PL011_INT_OEIM;

    // Enable UART, TX, RX
    uart->CR = PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE;

    // Clear pending GPU interrupts and enable UART IRQ (bit 25)
    gpu_interrupt->EnableIRQs2 &= ~UART_IRQ_BIT;
    gpu_interrupt->EnableIRQs2 |= UART_IRQ_BIT;
}

// --- Send character ---
void uart_putc(char input){
    while(uart->FR & PL011_FR_TXFF){}
    uart->DR = (uint32_t)input;
}

// --- Send string ---
void uart_puts(const char* string){
    while(*string != '\0'){
        uart_putc(*string++);
    }
}

// --- Get character (blocking) ---
char uart_getc(void) {
    // First try ring buffer (interrupt-driven)
    while(uart_rx_head == uart_rx_tail){
        // fallback polling
        if (!(uart->FR & PL011_FR_RXFE)) {
            return (char)uart->DR;
        }
    }

    char c = uart_rx_ring_buffer[uart_rx_tail];
    uart_rx_tail = (uart_rx_tail + 1) % UART_INPUT_BUFFER_SIZE;
    return c;
}

// --- Loopback (polling) ---
void uart_loopback(void){
    while(true){
        if (!(uart->FR & PL011_FR_RXFE)) {
            char c = (char)uart->DR;
            uart_putc(c);
        }
    }
}

void uart_irq_handler(void) {
    // Drain the FIFO
    while (!(uart->FR & PL011_FR_RXFE)) {
        char c = (char)uart->DR;
        unsigned int next_head = (uart_rx_head + 1) % UART_INPUT_BUFFER_SIZE;
        if (next_head != uart_rx_tail) {
            uart_rx_ring_buffer[uart_rx_head] = c;
            uart_rx_head = next_head;
        }
    }

    // Clear all interrupts
    uart->ICR = 0x7FF;  // Clears RX, TX, RT, OE, FE, PE, BE

    __asm volatile("dsb");  // Ensure memory operations complete
    __asm volatile("isb");  // Synchronize pipeline
}

