.section .init

.global _start
.global _setup_stacks

_setup_stacks:
    cpsid if                   /* disable interrupts */
    cps #0x13                  /* switch to SVC mode */
    ldr sp, =svc_stack_top     /* set SVC stack */
    push {lr}                  /* save return address */
    
    ldr r0, =irq_stack_top     /* load IRQ stack address */
    cps #0x12                  /* switch to IRQ mode */
    mov sp, r0                 /* set IRQ stack */
    mov lr, #0                 /* clear LR for exception modes */
    
    ldr r0, =abort_stack_top   /* load Abort stack address */
    cps #0x17                  /* switch to Abort mode */
    mov sp, r0                 /* set Abort stack */
    mov lr, #0                 /* clear LR */
    
    ldr r0, =und_stack_top     /* load Undefined stack address */
    cps #0x1b                  /* switch to Undefined mode */
    mov sp, r0                 /* set Undefined stack */
    mov lr, #0                 /* clear LR */
    
    ldr r0, =fiq_stack_top     /* load FIQ stack address */
    cps #0x11                  /* switch to FIQ mode */
    mov sp, r0                 /* set FIQ stack */
    mov lr, #0                 /* clear LR */
    
    ldr r0, =sys_stack_top     /* load System stack address */
    cps #0x1f                  /* switch to System mode */
    mov sp, r0                 /* set System stack */
    mov lr, #0                 /* clear LR */
    
    cps #0x13                  /* switch back to SVC mode */
    cpsie i                    /* enable interrupts */
    pop {lr}                   /* restore return address */
    bx lr                      /* return */

_start:
	/* Erkennen ob Hypervisor Mode aktiv */
	mrs r0, cpsr
	and r0, r0, #0x1F
	mov r1, #0x1A	//Modebits = 0x1A = Hypervisor Mode
	cmp r0, r1		//Vergleichen und Statusbits setzen für bedingte Sprünge

	/* Wenn Hypemode erkannt: abschalten (sprung nach _exitHyper)
	 * wenn nicht erkannt: dann weitermachen (weiterlaufen nach _bsprak)
	 */
	beq _exitHyper


_checkCores:
	/* Id des Cpu Cores Abfragen */
	mrc p15, 0, r0, c0, c0, 5
	/* Falls nicht core 0 Core disablen */
	tst r0, #3
	bne _parkCore

/* not modeled in qemu 6.0 */
_enableAlignCheck:
	/* SCTLR des cp15 laden */
	mrc p15, 0, r0, c1, c0, 0
	/* A-bit (c1[1]) auf 1 setzen */
	orr r0, r0, #0x2
	/* neues SCTLR speichern */
	mcr p15, 0, r0, c1, c0, 0


_bsprak:
    bl _setup_stacks           /* setup all stacks */
	ldr r0, =_ivt
	mcr p15, 0, r0, c12, c0, 0   
	bl start_kernel            /* jump to C code */
.Lend:
	WFI
	b .Lend

_parkCore:
	/* Interrupts für Core 1 bis 3 ausschalten  */
	cpsid if
	/* In Endlosschleife springen */
	b .Lend

_exitHyper:

	/* Rücksprungadresse ins Hypervisor Rücksprungregister schreiben.
	 * Dies ist das Label _bsprak für die normale Programmausfürung.
	 */
	ldr lr, =_checkCores
	msr ELR_hyp, lr

	/* Der Hypervisor mode ist auch "nur" eine etwas kompliziertere Exception.
	 * Also springen wir aus dieser zurück.
	 * (Rücksprunge in Aufgabe 2 sind anders zu lösen!).
	 * Dazu zunächst das CPSR auslesen und die Modebits zum Supervisor ändern.
	 * Danach dies in das SPSR für den Rücksprung schreiben.
	 */
	mrs r0, cpsr
	bic r0, r0, #0x1F
	orr r0, r0, #0x13	//Supervisormode
	msr spsr_hyp, r0
	eret
