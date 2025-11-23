
/*========================================================
  Generic exception handler macro for BCM2836 (ARMv7-A)
  Saves r0-r12, LR, SPSR safely and calls C handler
========================================================*/
.macro EXC_HANDLER name, lr_adjust=4
\name:
    sub lr, lr, #\lr_adjust       @ Adjust LR (4 for most, 8 for Data Abort)
    mrs r0, spsr                  @ Save SPSR first
    stmfd sp!, {r0}               @ Push SPSR
    stmfd sp!, {r0-r12, lr}       @ Push general-purpose registers + LR
    mov r0, sp                    @ Pass pointer to C handler
    bl \name\()_c                 @ Call C handler
    ldmfd sp!, {r0-r12, lr}       @ Restore registers + LR
    ldmfd sp!, {r0}               @ Restore SPSR
    msr spsr_fsx, r0
    movs pc, lr                   @ Return from exception, restores CPSR
.endm

/*--------------------------------------------------------
  Define exception handlers
--------------------------------------------------------*/
EXC_HANDLER software_interrupt       @ SWI/SVC
EXC_HANDLER irq
EXC_HANDLER fiq
EXC_HANDLER undefined_instruction
EXC_HANDLER prefetch_abort
EXC_HANDLER not_used

/* Data abort has a different LR adjustment (-8) */
EXC_HANDLER data_abort, 8

.section .ivt, "a"
.globl _ivt
.balign 64
_ivt:
	b reset
	ldr pc, _undefined_instruction
	ldr pc, _software_interrupt
	ldr pc, _prefetch_abort
	ldr pc, _data_abort
	ldr pc, _not_used
	ldr pc, _irq
	ldr pc, _fiq

_undefined_instruction: .word undefined_instruction
_software_interrupt: .word software_interrupt
_prefetch_abort: .word prefetch_abort
_data_abort: .word data_abort
_not_used: .word not_used
_irq: .word irq
_fiq: .word fiq


.globl reset
reset:
    /* Disable interrupts */
    cpsid i

    /* IRQ Mode Stack */
    cps #0x12              @ Switch to IRQ mode
    ldr sp, =irq_stack_top
    bic sp, sp, #7         @ 8-byte alignment

    /* Supervisor Mode Stack (Main Kernel Stack) */
    cps #0x13              @ SVC mode
    ldr sp, =svc_stack_top
    bic sp, sp, #7

    /* System Mode Stack (User Mode) */
    cps #0x1F              @ System mode
    ldr sp, =sys_stack_top
    bic sp, sp, #7

    /* Enable interrupts */
    cpsie i

    /* Jump to main() */
    bl main
