#include "arch/bsp/uart.h"
#include <lib/kprintf.h>
#include <stdarg.h>
#include <stdbool.h>
#include <lib/alib.h>
#include <stdint.h>
#define POINTER_STRING_LENGTH 11

// Main kprintf
void kprintf(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);

	while (*fmt) {
		if (*fmt == '%') {
			fmt++;
			char pad_char = ' ';
			int  width    = 0;

			if (*fmt == '0') {
				pad_char = '0';
				fmt++;
			}
			if (*fmt >= '0' && *fmt <= '9') {
				width = *fmt - '0';
				fmt++;
			}

			switch (*fmt++) {
			case 'c':
				uart_putc(va_arg(args, int));
				break;
			case 's':
				uart_puts(va_arg(args, const char *));
				break;
			case 'i':
				send_int_width(va_arg(args, int), width, pad_char);
				break;
			case 'u':
				send_unsigned_width(va_arg(args, unsigned int), width, pad_char,
						    false);
				break;
			case 'x':
				send_unsigned_width(va_arg(args, unsigned int), width, pad_char,
						    true);
				break;
			case 'p':
				send_pointer(va_arg(args, void *));
				break;
			case '%':
				uart_putc('%');
				break;
			default:
				uart_puts("Unknown conversion specifier");
			}
		} else {
			uart_putc(*fmt++);
		}
	}

	va_end(args);
}
// Helper: print padding
void print_padding(int width, char pad_char, int content_len)
{
	for (int i = content_len; i < width; i++)
		uart_putc(pad_char);
}

// Helper: convert int to string (decimal)
int int_to_str(int value, char *buffer)
{
	int	     len  = 0;
	unsigned int uval = value;
	if (value < 0)
		uval = -value;

	do {
		buffer[len++] = '0' + (uval % 10);
		uval /= 10;
	} while (uval > 0);

	if (value < 0)
		buffer[len++] = '-';

	// reverse
	for (int i = 0; i < len / 2; i++) {
		char tmp	    = buffer[i];
		buffer[i]	    = buffer[len - 1 - i];
		buffer[len - 1 - i] = tmp;
	}

	buffer[len] = '\0';
	return len;
}

int uint_to_str(unsigned int value, char *buffer)
{
    int len = 0;

    do {
        buffer[len++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    // reverse
    for (int i = 0; i < len / 2; i++) {
        char tmp = buffer[i];
        buffer[i] = buffer[len - 1 - i];
        buffer[len - 1 - i] = tmp;
    }

    buffer[len] = '\0';
    return len;
}

// Helper: convert unsigned int to lowercase hex string
int uint_to_hex_str(unsigned int value, char *buffer)
{
	int len = 0;
	if (value == 0) {
		buffer[len++] = '0';
		buffer[len]   = '\0';
		return len;
	}

	while (value > 0) {
		int digit = value % 16;
		if (digit < 10)
			buffer[len++] = '0' + digit;
		else
			buffer[len++] = 'a' + (digit - 10);
		value /= 16;
	}

	// reverse
	for (int i = 0; i < len / 2; i++) {
		char tmp	    = buffer[i];
		buffer[i]	    = buffer[len - 1 - i];
		buffer[len - 1 - i] = tmp;
	}

	buffer[len] = '\0';
	return len;
}

// Print integer with width and padding
void send_int_width(int value, int width, char pad_char)
{
	char buf[12];
	int  len = int_to_str(value, buf);

	if (pad_char == '0' && value < 0) {
		uart_putc('-'); // print minus first
		print_padding(width, pad_char, len); // subtract 1 because '-' already printed
		uart_puts(buf + 1); // skip the minus in the string
	} else {
		print_padding(width, pad_char, len);
		uart_puts(buf);
	}
}

// Print unsigned int / hex with width and padding
void send_unsigned_width(unsigned int value, int width, char pad_char, bool hex)
{
	char buf[12];
	int  len;
	if (hex)
		len = uint_to_hex_str(value, buf);
	else
		len = uint_to_str(value, buf); // for %u

	print_padding(width, pad_char, len);
	uart_puts(buf);
}

// Print pointer as 0xXXXXXXXX
void send_pointer(void *ptr)
{
	char	  buf[11]; // 0x + 8 digits + '\0'
	uintptr_t addr = (uintptr_t)ptr;
	buf[0]	       = '0';
	buf[1]	       = 'x';
	for (int i = 0; i < 8; i++) {
		int shift  = (7 - i) * 4;
		int digit  = (addr >> shift) & 0xF;
		buf[2 + i] = digit < 10 ? '0' + digit : 'a' + (digit - 10);
	}
	buf[10] = '\0';
	uart_puts(buf);
}

