.macro EXC_HANDLER name, lr_offset
\name:
    .if \lr_offset != 0
    sub lr, lr, #\lr_offset
    .endif
    
    @ Save r0-r12 and lr on exception mode stack
    stmfd sp!, {r0-r12, lr}
    
    @ Save user mode SP
    @ We need to switch to System mode to access user registers
    mrs r0, cpsr              @ Save current CPSR (exception mode)
    bic r1, r0, #0x1F         @ Clear mode bits
    orr r1, r1, #0x1F         @ Set to System mode (0x1F) - uses user registers
    msr cpsr_c, r1            @ Switch to System mode
    mov r2, sp                @ r2 = user SP
    msr cpsr_c, r0            @ Switch back to exception mode
    
    @ Push user SP onto exception stack
    stmfd sp!, {r2}
    
    @ Save SPSR
    mrs r0, spsr
    stmfd sp!, {r0}
    
    @ Call C handler with frame pointer
    mov r0, sp
    bl \name\()_c
    
    @ Restore SPSR
    ldmfd sp!, {r0}
    msr spsr_cxsf, r0
    
    @ Restore user SP
    ldmfd sp!, {r2}
    mrs r0, cpsr              @ Save current CPSR
    bic r1, r0, #0x1F         @ Clear mode bits
    orr r1, r1, #0x1F         @ System mode
    msr cpsr_c, r1            @ Switch to System mode
    mov sp, r2                @ Restore user SP
    msr cpsr_c, r0            @ Switch back to exception mode
    
    @ Restore r0-r12 and lr
    ldmfd sp!, {r0-r12, lr}
    movs pc, lr               @ Return and restore CPSR from SPSR
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
	b _start
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
