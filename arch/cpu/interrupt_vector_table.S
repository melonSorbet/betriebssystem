
/* arch/cpu/entry.S - Vector table + exception stubs for ARMv6 (RPi1)
   Produces a stack frame: r0..r12, lr, spsr
*/

    .text
    .syntax unified
    .arch armv6
    .fpu softvfp

    .equ VECTOR_TABLE_ADDRESS, 0x00000000

    .macro EXC_HANDLER name, lr_adjust
\name:
    sub     lr, lr, #\lr_adjust    @ adjust LR (PC + offset -> PC of faulting instr + ?)
    stmfd   sp!, {r0-r12, lr}      @ save general regs + LR (banked LR)
    mrs     r0, SPSR               @ copy SPSR into r0
    stmfd   sp!, {r0}              @ push SPSR copy (so frame->spsr valid)
    mov     r0, sp                 @ r0 = pointer to frame
    bl      \name\()_c             @ call C handler
    ldmfd   sp!, {r0}              @ pop/discard SPSR copy
    ldmfd   sp!, {r0-r12, pc}^     @ restore regs and return (restore CPSR)
    .endm

    .macro EXC_HANDLER_NOADJ name
\name:
    stmfd   sp!, {r0-r12, lr}
    mrs     r0, SPSR
    stmfd   sp!, {r0}
    mov     r0, sp
    bl      \name\()_c
    ldmfd   sp!, {r0}
    ldmfd   sp!, {r0-r12, pc}^
    .endm

    /* Vector table (must be at 0x0). Align to 64 bytes for ARM vector table */
    .section .ivt, "a"
    .balign 64
    .global _ivt
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
_software_interrupt:     .word software_interrupt
_prefetch_abort:         .word prefetch_abort
_data_abort:             .word data_abort
_not_used:               .word not_used
_irq:                    .word irq
_fiq:                    .word fiq

    .text
    .global reset
reset:
    cpsid   i                  @ disable IRQs

    /* Setup stacks for modes. Use fixed symbols defined in a linker script or .bss */
    /* IRQ stack */
    cps     #0x12              @ switch to IRQ mode
    ldr     sp, =irq_stack_top
    bic     sp, sp, #7

    /* Supervisor (SVC) stack */
    cps     #0x13
    ldr     sp, =svc_stack_top
    bic     sp, sp, #7

    /* System/User (use system) stack */
    cps     #0x1F
    ldr     sp, =sys_stack_top
    bic     sp, sp, #7

    cpsie   i                  @ enable IRQs
    bl      main

/* Exception stubs - use EXC_HANDLER macro with appropriate LR adjustment:
   For most exceptions LR points to the next instruction (PC + 4) -> subtract 4.
   For data abort LR is PC + 8 -> subtract 8.
*/
    .text
    EXC_HANDLER software_interrupt, 4
    EXC_HANDLER irq, 4
    EXC_HANDLER fiq, 4
    EXC_HANDLER undefined_instruction, 4
    EXC_HANDLER prefetch_abort, 4
    EXC_HANDLER not_used, 4

/* data_abort needs a different LR adjustment */
data_abort:
    sub     lr, lr, #8
    stmfd   sp!, {r0-r12, lr}
    mrs     r0, SPSR
    stmfd   sp!, {r0}
    mov     r0, sp
    bl      data_abort_c
    ldmfd   sp!, {r0}
    ldmfd   sp!, {r0-r12, pc}^

/* Stacks (ensure sizes and symbols exist in linker script or adjust names) */
    .bss
    .align 7
    .global irq_stack_top
irq_stack_space:
    .skip 4096
irq_stack_top:

    .align 7
    .global svc_stack_top
svc_stack_space:
    .skip 8192
svc_stack_top:

    .align 7
    .global sys_stack_top
sys_stack_space:
    .skip 8192
sys_stack_top:

