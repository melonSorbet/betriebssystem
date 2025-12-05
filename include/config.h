#ifndef CONFIG_H_
#define CONFIG_H_

/* Falls auf den Boards gearbeitet wird am besten
 * die nächste Zeile auskommentieren
 */
#define BUILD_FOR_QEMU

#ifndef __ASSEMBLER__

/**
 * \file config.h
 *
 * Enthält defines und static Funktionen zum testen der
 * Implementierung. Wir tauschen diese Werte/Funktionen beim
 * Korrigieren zum Testen aus. Bitte fügt hier KEINE weiteren
 * defines oÄ ein. Ihr könnt diese Werte zum Testen natürlich
 * auch gerne selbst verändern.
 */

void test_kernel [[gnu::weak]] (void);
void test_user [[gnu::weak]] (void *args);
void test_user_main [[gnu::weak]] (void);

/**
 * Erst ab Aufgabenblatt 2 relevant
 */

#ifdef BUILD_FOR_QEMU
/* Werte zum testen unter QEMU */
static constexpr unsigned int BUSY_WAIT_COUNTER = 10000000;
#else
/* Werte zum testen auf der Hardware */
static constexpr unsigned int BUSY_WAIT_COUNTER = 30000;
#endif // BUILD_FOR_QEMU

static constexpr unsigned int PRINT_COUNT = 5;

// Wir testen nur mit Werten die durch 2^n darstellbar sind
static constexpr unsigned int UART_INPUT_BUFFER_SIZE = 128;

// Timer Interrupt Interval zum testen in Mikrosekunden
// Systimer taktet mit 1MHz
// 1000000 -> 1 Sekunde
static constexpr unsigned int TIMER_INTERVAL = 1000000;

#endif // __ASSEMBLER__
#endif // CONFIG_H_
