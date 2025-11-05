#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

/**
 * \brief Ringbuffer Datenstruktur
 * \file ringbuffer.h
 *
 * Diese Datei ist eine Hilfestellung für Aufgabe 2.
 * Sie erspart euch den Aufwand, selber einen Ringbuffer zu implementieren.
 */

struct ring_buff {
	unsigned int head;
	unsigned int tail;
	unsigned int size;
	unsigned int mask;
	char	    *buffer;
};

/**
 * \brief Überprüft ob eine positive Zahl eine Zweierpotenz ist
 *
 * Beispiel: (0b100 & 0b011) == 0b000.
 */
#define is_power_of_two(val) (((val) & ((val) - 1)) == 0)

/**
 * \brief Macro um einen Ringbuffer zu erstellen
 * \param name Bezeichnung des Buffers
 * \param size Größe des Buffers
 *
 */
#define create_ringbuffer(name, size)                                                        \
	static_assert((size) >= 1, "Size of Ringbuffer has to be at least 1");               \
	static_assert(is_power_of_two(size), "Size of Ringbuffer has to be a power of 2");   \
	static char			  _b_##name[size];                                   \
	static struct ring_buff		  _##name = { 0, 0, (size), (size) - 1, _b_##name }; \
	static volatile struct ring_buff *name	  = &_##name

[[nodiscard]] static bool buff_is_empty(volatile struct ring_buff *b)
{
	return b->head == b->tail;
}

[[nodiscard]] static bool buff_is_full(volatile struct ring_buff *b)
{
	return ((b->tail & b->mask) == (b->head & b->mask)) && !buff_is_empty(b);
}

static bool buff_putc(volatile struct ring_buff *b, char c)
{
	if (buff_is_full(b)) {
		return true;
	}

	b->buffer[b->head & b->mask] = c;
	b->head++;

	return false;
}

static char buff_getc(volatile struct ring_buff *b)
{
	while (buff_is_empty(b)) {
	}

	char c = b->buffer[b->tail & b->mask];
	b->tail++;

	return c;
}

[[nodiscard, maybe_unused]] static char buff_peekc(volatile struct ring_buff *b)
{
	while (buff_is_empty(b)) {
	}

	return b->buffer[b->tail & b->mask];
}

[[nodiscard, maybe_unused]] static char buff_peekc_last(volatile struct ring_buff *b)
{
	while (buff_is_empty(b)) {
	}

	return b->buffer[(b->head - 1) & b->mask];
}

#endif
