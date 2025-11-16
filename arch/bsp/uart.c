#ifndef UART_INPUT_BUFFER_SIZE
#define UART_INPUT_BUFFER_SIZE 256 // Default size if not defined in config.h
#endif

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h> // For NULL
#include <lib/kprintf.h>

#define PL011_BUS_BASE 0x7E201000
#define BUS_BASE 0x7E000000
#define CPU_PERIPHERAL_BASE 0x3F000000

// Calculate the ARM physical address for the UART
#define PL011_BASE ((PL011_BUS_BASE - BUS_BASE) + CPU_PERIPHERAL_BASE)
typedef struct {
    volatile uint32_t DR;          // 0x00 Data Register
    volatile uint32_t RSR_ECR;     // 0x04 Receive Status Register / Error Clear Register
    volatile uint32_t RESERVED0[4];// 0x08, 0x0C, 0x10, 0x14 - These 4x 32-bit words are reserved/unused
    volatile uint32_t FR;          // 0x18 Flag Register
    volatile uint32_t ILPR;         // 0x1C - This is where ILPR *would* be, but you've confirmed it's "not in use". Treat as reserved.
    volatile uint32_t RESERVED2;   // 0x20 - Another reserved register (fills the gap between 0x1C and 0x24)
    volatile uint32_t IBRD;        // 0x24 Integer Baud Rate Divisor
    volatile uint32_t FBRD;        // 0x28 Fractional Baud Rate Divisor
    volatile uint32_t LCRH;        // 0x2C Line Control register
    volatile uint32_t CR;          // 0x30 Control register
    volatile uint32_t IFLS;        // 0x34 Interrupt FIFO Level Select Register
    volatile uint32_t IMSC;        // 0x38 Interrupt Mask Set Clear Register
    volatile uint32_t RIS;         // 0x3C Raw Interrupt Status Register
    volatile uint32_t MIS;         // 0x40 Masked Interrupt Status Register
    volatile uint32_t ICR;         // 0x44 Interrupt Clear Register
    volatile uint32_t DMACR;       // 0x48 DMA Control Register    // If DMACR is at 0x48, add it here.
} UART;

static volatile UART* const uart = (UART*)(PL011_BASE);
// Flag Register (FR) bits
#define PL011_FR_TXFF   (1 << 5) // Transmit FIFO Full
#define PL011_FR_RXFE   (1 << 4) // Receive FIFO Empty
#define PL011_FR_BUSY   (1 << 3) // UART Busy

// Line Control Register (LCRH) bits
#define PL011_LCRH_WLEN_8BIT (3 << 5) // 8-bit word length
#define PL011_LCRH_FEN  (1 << 4) // FIFOs Enable

// Control Register (CR) bits
#define PL011_CR_RXE    (1 << 9)  // Receive Enable
#define PL011_CR_TXE    (1 << 8)  // Transmit Enable
#define PL011_CR_UARTEN (1 << 0)  // UART Enable

// Interrupt Mask Set/Clear Register (IMSC) bits & Interrupt Clear Register (ICR) bits
#define PL011_INT_OEIM  (1 << 1)  // Overrun Error Interrupt Mask
#define PL011_INT_RTIM  (1 << 6)  // Receive Timeout Interrupt Mask
#define PL011_INT_RXIM  (1 << 4)  // Receive Interrupt Mask (for new data)

// --- ARM Local Interrupt Controller (on the CPU core) ---
#define LOCAL_PERIPHERAL_BASE 0x40000000
#define CORE0_IRQ_SOURCE ((volatile uint32_t*)(LOCAL_PERIPHERAL_BASE + 0x60)) // ARM Interrupt Controller - IRQ Pending register

// --- BCM283x GPU Interrupt Controller ---
#define BUS_BASE 0x7E000000
#define CPU_PERIPHERAL_BASE 0x3F000000
#define GPU_INTERRUPTS_BUS_BASE 0x7E00B000
#define GPU_INTERRUPTS_CPU_PHYSICAL (((GPU_INTERRUPTS_BUS_BASE - BUS_BASE) + CPU_PERIPHERAL_BASE) + 0x200)

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

static volatile GPU_Interrupt_Controller* const gpu_interrupt = (GPU_Interrupt_Controller*)(GPU_INTERRUPTS_CPU_PHYSICAL);


// --- Ring Buffer for Received Characters ---
// This buffer is where interrupt-received characters are stored.
static volatile char uart_rx_ring_buffer[UART_INPUT_BUFFER_SIZE];
static volatile unsigned int uart_rx_head = 0; // Index for writing new data (producer)
static volatile unsigned int uart_rx_tail = 0; // Index for reading data (consumer)

// --- Function Prototypes ---


#define UART_IMSC       (*(volatile unsigned int*)(PL011_BASE + 0x038))
#define UART_CR         (*(volatile unsigned int*)(PL011_BASE + 0x030))
#define UART_ICR        (*(volatile unsigned int*)(PL011_BASE + 0x044))

#define UART_RXIM       (1 << 4)  // Receive interrupt
#define UART_TXIM       (1 << 5)  // Transmit interrupt
#define UART_RTIM       (1 << 6)  
// --- UART Initialization Function ---
#define UART_IRQ_BIT     (1 << 25) // PL011 UART IRQ is bit 57 → 57-32 = 25

// Konfiguriert den PL011 so, dass dieser Zeichen Interrupt-gesteuert empfängt.
void uart_init(void) {
    gpu_interrupt->EnableIRQs2 = UART_IRQ_BIT;
    uart->CR = 0x00000000;
    kprintf("adress of IMSC %x", &uart->IMSC);
    uart->ICR = 0x7FF;
    uart->IMSC |= (1 << 4);
    uart->IMSC |= PL011_INT_RTIM;
    uart->IMSC |= PL011_INT_OEIM; 
    *CORE0_IRQ_SOURCE |= (1 << 8); 
}

void uart_putc(char input){
    while(uart->FR & PL011_FR_TXFF){}

    uart->DR = (uint32_t)input;
}


char uart_getc(void){
    
    while (uart_rx_head == uart_rx_tail) {
    }

    char c = uart_rx_ring_buffer[uart_rx_tail];

    uart_rx_tail = (uart_rx_tail + 1) % UART_INPUT_BUFFER_SIZE;

    return c;
}

// --- UART Put String Function ---
void uart_puts(const char* string){
    while(*string != '\0'){
        uart_putc(*string);
        string++;
    }
}

// --- UART Loopback Function ---
void uart_loopback [[noreturn]] (void){
    while (true) {
        if (!(uart->FR & PL011_FR_RXFE)) {
            char c = (char)uart->DR;
            uart_putc(c);
        }
    }
}


