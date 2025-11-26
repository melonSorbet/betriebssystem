.macro EXC_HANDLER name, lr_offset
\name:
    .if \lr_offset != 0
    sub lr, lr, #\lr_offset
    .endif
    stmfd sp!, {r0-r12, lr}
    mrs r0, spsr
    stmfd sp!, {r0}
    mov r0, sp
    bl \name\()_c
    ldmfd sp!, {r0}
    msr spsr_cxsf, r0
    ldmfd sp!, {r0-r12, lr}
    movs pc, lr              @ 's' restores CPSR from SPSR
.endm

EXC_HANDLER software_interrupt, 0
EXC_HANDLER irq, 4
EXC_HANDLER fiq, 4
EXC_HANDLER undefined_instruction, 0
EXC_HANDLER prefetch_abort, 4
EXC_HANDLER data_abort, 8
EXC_HANDLER not_used, 0

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
    cpsid if
    
    cps #0x12
    ldr sp, =irq_stack_top
    bic sp, sp, #7
    
    cps #0x17
    ldr sp, =abort_stack_top
    bic sp, sp, #7
    
    cps #0x1B
    ldr sp, =und_stack_top
    bic sp, sp, #7
    
    cps #0x13
    ldr sp, =svc_stack_top
    bic sp, sp, #7
    
    cps #0x1F
    ldr sp, =sys_stack_top
    bic sp, sp, #7
    
    cps #0x13
    cpsie if
    
    bl main
