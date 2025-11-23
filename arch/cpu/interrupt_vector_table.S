.section .init

.macro exception_trampoline handler_name, return_offset, c_handler
\handler_name:
    @ Save all registers in the exact order of exc_frame_t structure
    stmfd sp!, {r0-r12, lr}      @ r0-r12, lr (16 registers total)
    
    @ Save SPSR (goes after all the general purpose registers)
    mrs r0, spsr
    stmfd sp!, {r0}              @ spsr (17th register)
    
    @ Adjust LR for return if needed
    .if \return_offset != 0
    sub lr, lr, #\return_offset
    .endif
    
    @ Call the C handler with stack pointer as context argument
    mov r0, sp
    bl \c_handler
    
    @ Restore SPSR and all registers
    ldmfd sp!, {r0}              @ spsr
    msr spsr, r0
    ldmfd sp!, {r0-r12, lr}      @ r0-r12, lr
    
    @ Return from exception (restores CPSR from SPSR)
    movs pc, lr
.endm

.global _ivt
.balign 64
_ivt:
    b _start
    ldr pc, _undefined_instruction
    ldr pc, _supervisor_call
    ldr pc, _prefetch_abort
    ldr pc, _data_abort
    ldr pc, _not_used
    ldr pc, _irq
    ldr pc, _fiq

_undefined_instruction: .word undefined_instruction_trampoline
_supervisor_call: .word supervisor_call_trampoline
_prefetch_abort: .word prefetch_abort_trampoline
_data_abort: .word data_abort_trampoline
_not_used: .word not_used_trampoline
_irq: .word irq_trampoline
_fiq: .word fiq_trampoline

@ Trampoline definitions with correct offsets:
exception_trampoline supervisor_call_trampoline, 0, software_interrupt_c
exception_trampoline undefined_instruction_trampoline, 4, undefined_instruction_c
exception_trampoline prefetch_abort_trampoline, 4, prefetch_abort_c  
exception_trampoline data_abort_trampoline, 8, data_abort_c
exception_trampoline not_used_trampoline, 0, not_used_c
exception_trampoline irq_trampoline, 4, irq_c
exception_trampoline fiq_trampoline, 4, fiq_c
