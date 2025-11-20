.macro EXC_HANDLER name
\name:
    sub lr, lr, #4        @ Adjust LR for prefetch abort
    stmfd sp!, {r0-r12, lr}
    mov r0, sp
    bl \name\()_c
    ldmfd sp!, {r0-r12, pc}^  @ ^ restores CPSR
.endm

EXC_HANDLER software_interrupt
EXC_HANDLER irq
EXC_HANDLER fiq
EXC_HANDLER undefined_instruction
EXC_HANDLER prefetch_abort
EXC_HANDLER not_used

/* Separate Data Abort handler for LR fixup */
data_abort:
    sub lr, lr, #8
    stmfd sp!, {r0-r12, lr}
    mov r0, sp
    bl data_abort_c
    ldmfd sp!, {r0-r12, pc}^



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
