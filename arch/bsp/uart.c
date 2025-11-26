
#ifndef UART_INPUT_BUFFER_SIZE
#define UART_INPUT_BUFFER_SIZE 256
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <lib/kprintf.h>
#include "arch/bsp/uart.h"

#define PL011_BUS_BASE           0x7E201000
#define BUS_BASE                 0x7E000000
#define CPU_PERIPHERAL_BASE      0x3F000000
#define PL011_BASE               ((PL011_BUS_BASE - BUS_BASE) + CPU_PERIPHERAL_BASE)

// GPIO
#define GPIO_BASE   (CPU_PERIPHERAL_BASE + 0x200000)
#define GPFSEL1     ((volatile uint32_t*)(GPIO_BASE + 0x04))

#define TXD_PIN 14
#define RXD_PIN 15
#define TXD_PIN_INDEX (TXD_PIN % 10)
#define RXD_PIN_INDEX (RXD_PIN % 10)
#define GPIO_FUNC_BITS 3
#define TXD_SHIFT (TXD_PIN_INDEX * GPIO_FUNC_BITS)
#define RXD_SHIFT (RXD_PIN_INDEX * GPIO_FUNC_BITS)
#define GPIO_FUNC_MASK 0x7
#define GPIO_ALT0 0x4

// UART registers
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
bool irq_debug = false;
volatile GPU_Interrupt_Controller* const gpu_interrupt =
    (GPU_Interrupt_Controller*)0x3F00B200;

static volatile UART* const uart = (UART*)(PL011_BASE);

#define UART_IRQ_BIT (1 << 25)

// Flags
#define PL011_FR_TXFF   (1 << 5)
#define PL011_FR_RXFE   (1 << 4)

// Line control
#define PL011_LCRH_WLEN_8BIT (3 << 5)
#define PL011_LCRH_FEN  (1 << 4)

// Control
#define PL011_CR_RXE    (1 << 9)
#define PL011_CR_TXE    (1 << 8)
#define PL011_CR_UARTEN (1 << 0)

// Interrupt bits
#define PL011_INT_RXIM  (1 << 4)
#define PL011_INT_RTIM  (1 << 6)
#define PL011_INT_OEIM  (1 << 1)


// =============================================================
//                      RX RING BUFFER
// =============================================================
static volatile char uart_rx_ring_buffer[UART_INPUT_BUFFER_SIZE];
static volatile unsigned int uart_rx_head = 0;
static volatile unsigned int uart_rx_tail = 0;


// =============================================================
//                       UART INIT
// =============================================================
void uart_init(void) {

    // ---------------- GPIO ALT0 ----------------
    uint32_t gpfsel1_val = *GPFSEL1;
    gpfsel1_val &= ~(GPIO_FUNC_MASK << TXD_SHIFT);
    gpfsel1_val &= ~(GPIO_FUNC_MASK << RXD_SHIFT);
    gpfsel1_val |= (GPIO_ALT0 << TXD_SHIFT);
    gpfsel1_val |= (GPIO_ALT0 << RXD_SHIFT);
    *GPFSEL1 = gpfsel1_val;

    // ---------------- Disable UART ----------------
    uart->CR = 0;
    uart->ICR = 0x7FF;  // clear interrupts

    // ---------------- Set baud rate ----------------
    // 48 MHz UART clock → 115200 baud
    uart->IBRD = 26;
    uart->FBRD = 3;

    // ---------------- 8-bit + FIFO -----------------
    uart->LCRH = PL011_LCRH_WLEN_8BIT | PL011_LCRH_FEN;

    // ---------------- Enable RX interrupts ---------
    uart->IMSC = PL011_INT_RXIM | PL011_INT_RTIM | PL011_INT_OEIM;

    // ---------------- Enable UART ------------------
    uart->CR = PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE;

    // ---------------- Enable IRQ in GPU ------------
    gpu_interrupt->EnableIRQs2 |= UART_IRQ_BIT;
}



// =============================================================
//                  UART TX ROUTINES
// =============================================================
void uart_putc(char input){
    while(uart->FR & PL011_FR_TXFF){}
    uart->DR = (uint32_t)input;
}

void uart_puts(const char* string){
    while(*string)
        uart_putc(*string++);
}



// =============================================================
//                  UART RX ROUTINES
// =============================================================

// --- Fully interrupt-driven, blocking ---
char uart_getc(void) {
    while (uart_rx_head == uart_rx_tail) {
        asm volatile("wfi");  // wait for interrupt
    }
    char c = uart_rx_ring_buffer[uart_rx_tail];
    uart_rx_tail = (uart_rx_tail + 1) % UART_INPUT_BUFFER_SIZE;
    return c;
}

// --- Non-blocking, returns -1 if empty ---
int uart_try_getc(void) {
    if (uart_rx_head == uart_rx_tail)
        return -1;
    char c = uart_rx_ring_buffer[uart_rx_tail];
    uart_rx_tail = (uart_rx_tail + 1) % UART_INPUT_BUFFER_SIZE;
    return c;
}



// =============================================================
//                       IRQ HANDLER
// =============================================================
void uart_irq_handler(void) {

    // Drain RX FIFO
    while (!(uart->FR & PL011_FR_RXFE)) {
        char c = (char)uart->DR;

        unsigned int next_head = (uart_rx_head + 1) % UART_INPUT_BUFFER_SIZE;

        // drop char if ring buffer full
        if (next_head != uart_rx_tail) {
            uart_rx_ring_buffer[uart_rx_head] = c;
            uart_rx_head = next_head;
        }
    }

    // Clear interrupt sources
    uart->ICR = 0x7FF;

    asm volatile("dsb");
    asm volatile("isb");
}

