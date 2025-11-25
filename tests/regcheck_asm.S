
#include <config.h>

/* (bedingt ausgeführte) Undefined Instructions... yay */
#define undef .word 0xe7ffffff
#define undefne .word 0x17ffffff
#define undefeq .word 0x07ffffff

#ifdef BUILD_FOR_QEMU
	#define wait_count_1 #0x80000000
	#define wait_count_2 #0x0A000000
#else
	#define wait_count_1 #0x00800000
	#define wait_count_2 #0x01000000
#endif



#undef  PSR_SYS
#define PSR_SYS (0b11111)
#undef  PSR_ABT
#define PSR_ABT (0b10111)
#undef  XOR_SYS_ABT
#define XOR_SYS_ABT (PSR_SYS ^ PSR_ABT)
#undef  PSR_I
#define PSR_I (1<<7)
#undef  PSR_F
#define PSR_F (1<<6)
#undef  PSR_V
#define PSR_V (1<<28)

/* Kann als backup genutzt werden, falls Qemu undefne/undefeq nicht
 * wie erwartet emuliert, da diese laut Arm undefiniert sind
 * undefne -> bne _check_throw_undef
 * undefeq -> beq _check_throw_undef
 */
_check_throw_undef:
	undef
	b _check_throw_undef


/*
 * _check_return_jump() - erkennt falsche Return-Offsets von -24 bis +24
 *
 * Im Bereich -12 bis +12 stimmt die Diagnose hier im Quellcode.
 *
 * Nur der Fall "4 zu spät" wird pro Interrupt gelegentlich verpasst.
 * Mit genügend Interrupts während der Ausführung sinkt die
 * Wahrscheinlichkeit entsprechend.
 *
 * Bei Detektion wird eine Undefined-Instruction-Ausnahme ausgelöst.
 * Wenn ihr dort das korrekte Offset ausgebt, könnt ihr per objdump
 * rausfinden, in welche Richtung euer Rücksprung verkehrt ist.
 *
 *
 * Funktionsweise: Es wird ständig zwischen zwei/drei Instruktionen
 * hin und her gesprungen, die von Undefined Instructions umgeben sind.
 */
.global _check_return_jump
_check_return_jump:
	mov	r0, wait_count_1
	b	1f
	undef
	undef
	undef
	undef	/* 12 zu früh */
	undef	/* 8 zu früh */
	undef	/* 4 zu früh */
2:
	b	1f
	undef	/* 4 zu spät */
	undef	/* 8 zu spät */
	undef	/* 12 zu spät */
	undef	/* 12 zu früh */
	undef	/* 8 zu früh */
	undef	/* 4 zu früh */
1:
	subs	r0, r0, #1
	bne	2b		/* richtig oder 4 zu spät, wird nicht erwischt, springt nach oben */
	bxeq	lr		/* 4 oder 8 zu spät, fall-through auf nächstes undef */
	undef			/* 4 oder 8 zu spät (fall-through) bzw. 8 oder 12 zu spät */
	undef			/* 12 oder 16 zu spät */
	undef
	undef
	undef

/*
 * _check_registers() - erkennt Änderungen an r0-r14
 *
 * Unter der Annahme, dass an die richtige Stelle zurückgesprungen wird,
 * werden die meisten Änderungen an r0-r14 aufgedeckt.
 *
 * Bei Detektion wird eine Undefined-Instruction-Ausnahme ausgelöst.
 * Der Ort zusammen mit einem Registerdump sollte Aufschluss über die
 * fehlerhaften Register geben.
 *
 *
 * Funktionsweise: Register r0-r14 werden gleichmäßig über den gesamten
 * Wertebereich hochgezählt. Während des Hochzählens werden die Register
 * miteinander verglichen, mal wenn sich identisch sein sollten, mal wenn sie
 * unterschiedlich sein sollten.
 */
.global _check_registers
_check_registers:
#undef STEP
#define STEP (452)
#undef END
#define END (0xfffffff0)
#if (END % STEP)
#error "END needs to be a whole multiple of STEP"
#endif
	push	{r4-r11, lr}
	mov	r0, #0
	mov	r7, r0
	mov	r1, r8
	mov	r2, r9
	mov	r3, r10
	mov	r4, r11
	mov	r5, r12
	mov	r6, r13
	mov	r14, r13
1:
	add	r0, r0, #STEP
	add	r1, r1, #STEP
	add	r2, r2, #STEP
	add	r3, r3, #STEP
	add	r4, r4, #STEP
	add	r5, r5, #STEP
	add	r6, r6, #STEP
	cmp	r0, r7
	undefeq			/* r0 oder r7 falsch */
	cmp	r1, r8
	undefeq			/* r1 oder r8 falsch */
	cmp	r2, r9
	undefeq			/* r2 oder r9 falsch */
	cmp	r3, r10
	undefeq			/* r3 oder r10 falsch */
	cmp	r4, r11
	undefeq			/* r4 oder r11 falsch */
	cmp	r5, r12
	undefeq			/* r5 oder r12 falsch */
	cmp	r6, r14
	undefeq			/* r6 oder r14 falsch */
	cmp	r6, r13
	undefeq			/* r13 falsch */
	add	r7, r7, #STEP
	add	r8, r8, #STEP
	add	r9, r9, #STEP
	add	r10, r10, #STEP
	add	r11, r11, #STEP
	add	r12, r12, #STEP
	add	r13, r13, #STEP
	add	r14, r14, #STEP
	cmp	r0, r7
	undefne			/* r0 oder r7 falsch */
	cmp	r1, r8
	undefne			/* r1 oder r8 falsch */
	cmp	r2, r9
	undefne			/* r2 oder r9 falsch */
	cmp	r3, r10
	undefne			/* r3 oder r10 falsch */
	cmp	r4, r11
	undefne			/* r4 oder r11 falsch */
	cmp	r5, r12
	undefne			/* r5 oder r12 falsch */
	cmp	r6, r14
	undefne			/* r6 oder r14 falsch */
	cmp	r6, r13
	undefne			/* r13 falsch */

	cmp	r0, #END
	blo	1b

	/* SP rückrechnen */
	sub	sp, sp, #END

	pop	{r4-r11, lr}
	bx	lr


/*
 * _check_cpsr() - Änderungen an CPSR detektieren
 *
 * Bis auf das T-Bit werden alle Flags und der Modus ständig verändert.
 * (Deshalb muss die Funktion von einem priviligierten Modus aus aufgerufen
 * werden.)
 *
 * Bei Detektion wird eine Undefined-Instruction-Ausnahme ausgelöst.
 */
.global _check_cpsr
_check_cpsr:
	push	{r4}

	/*
	 * r0: Schleifenzähler
	 * r1: tmp
	 * r2: tmp
	 * r3: neuer Wert für CPSR
	 * r4: Backup von CPSR
	 */
	mov	r0, #0
	mrs	r4, cpsr
	mov	r3, r4

	/* Im System-Modus anfangen */
	orr	r3, r3, #PSR_SYS
1:
	/* CPSR setzen und IRQ eine Chance geben */
	msr	cpsr, r3
	mov	r1, r3
	add	r3, r3, #PSR_V	/* jedesmal Flags ändern */
	add	r0, r0, #1

	/* CPSR lesen und mit altem Wert vergleichen */
	mrs	r2, cpsr
	cmp	r2, r1
	undefne			/* cpsr verfälscht */

	/* jedes 16. mal I ändern */
	ands	r2, r0, #(1<<4)
	eoreq	r3, r3, #PSR_I

	/* jedes 32. mal F ändern */
	ands	r2, r0, #(1<<5)
	eoreq	r3, r3, #PSR_F

	/* jedes 64. mal Modus ändern (System <-> Abort)*/
	ands	r2, r0, #(1<<6)
	eoreq	r3, r3, #XOR_SYS_ABT

	cmp	r0, wait_count_2
	bne	1b

	msr	cpsr, r4
	pop	{r4}
	bx	lr


/*
 * _check_spsr() - Änderungen an SPSR detektieren
 *
 * Bis auf das verwendete Register identisch zu _check_cpsr().
 */
.global _check_spsr
_check_spsr:
	push	{r4}

	/*
	 * r0: Schleifenzähler
	 * r1: tmp
	 * r2: tmp
	 * r3: neuer Wert für SPSR
	 * r4: Backup von SPSR
	 */
	mov	r0, #0
	mrs	r4, spsr
	mov	r3, r4

	/* Im System-Modus anfangen */
	orr	r3, r3, #PSR_SYS
1:
	/* SPSR setzen und IRQ eine Chance geben */
	msr	spsr, r3
	mov	r1, r3
	add	r3, r3, #PSR_V	/* jedesmal Flags ändern */
	add	r0, r0, #1

	/* SPSR lesen und mit altem Wert vergleichen */
	mrs	r2, spsr
	cmp	r2, r1
	undefne			/* spsr verfälscht */

	/* jedes 16. mal I ändern */
	ands	r2, r0, #(1<<4)
	eoreq	r3, r3, #PSR_I

	/* jedes 32. mal F ändern */
	ands	r2, r0, #(1<<5)
	eoreq	r3, r3, #PSR_F

	/* jedes 64. mal Modus ändern (System <-> Abort)*/
	ands	r2, r0, #(1<<6)
	eoreq	r3, r3, #XOR_SYS_ABT

	cmp	r0, wait_count_2
	bne	1b

	msr	spsr, r4
	pop	{r4}
	bx	lr
