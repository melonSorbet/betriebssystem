#include <arch/bsp/uart.h>
#include <stdint.h>
#define PL011_BUS_BASE 0x7E201000
#define BUS_BASE 0x7E000000
#define CPU_PERIPHERAL_BASE 0x3F000000

#define PL011_BASE ((PL011_BUS_BASE - BUS_BASE) + CPU_PERIPHERAL_BASE)

struct pl011{
	volatile unsigned int data_register;
	volatile unsigned int RSRECR;
	volatile unsigned int reserved[4];
	volatile unsigned int FR;

};

#define UART_DR     (*(volatile uint32_t*)(PL011_BASE + 0x00))
#define UART_FR     (*(volatile uint32_t*)(PL011_BASE + 0x18))
#define UART_IBRD   (*(volatile uint32_t*)(PL011_BASE + 0x24))
#define UART_FBRD   (*(volatile uint32_t*)(PL011_BASE + 0x28))
#define UART_LCRH   (*(volatile uint32_t*)(PL011_BASE + 0x2C))
#define UART_CR     (*(volatile uint32_t*)(PL011_BASE + 0x30))
#define UART_IMSC   (*(volatile uint32_t*)(PL011_BASE + 0x38))
#define UART_ICR    (*(volatile uint32_t*)(PL011_BASE + 0x44))

#define PL011_FR_TXFF (1 << 5)   // Transmit FIFO Full
#define PL011_FR_RXFE (1 << 4)

static volatile struct pl011 *const pl011_port = (struct pl011 *)PL011_BASE;


void setup_uart(){
	UART_CR = (1 << 9) | (1 << 8) | (1 << 0);  
}
void uart_loopback [[noreturn]] (void){
	while(true){
		uart_putc(uart_getc());
	}
}

void uart_putc(char input) {
    while (pl011_port->FR & PL011_FR_TXFF) {
        // Wait until not full
    }
    pl011_port->data_register = (unsigned int)input;
}

char uart_getc(void) {
    while (pl011_port->FR & PL011_FR_RXFE) {
        // Wait until data available
    }
    return (char)(pl011_port->data_register & 0xFF);
}

void uart_puts(const char* string){
	while(*string != '\0'){
		uart_putc(*string);
		string++;
	}
}
