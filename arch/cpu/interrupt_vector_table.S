.section .init

.macro exception_trampoline handler_name, return_offset, c_handler
\handler_name:
	.if \return_offset != 0
	sub lr, lr, #\return_offset @ For IRQ/FIQ: adjust LR to interrupted + 4
	.endif
	
	@ Save minimal context: all GP regs + LR (exception return address)
	stmfd sp!, {r0-r12, lr}
	
	@ Save SPSR (the key SPR you need)
	mrs r0, spsr
	stmfd sp!, {r0}
	
	@ Call the specific C handler with stack pointer as context argument
	mov r0, sp
	bl \c_handler
	
	@ Restore SPSR and context
	ldmfd sp!, {r0}
	msr spsr, r0
	ldmfd sp!, {r0-r12, lr}
	
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

@ Now with the correct C handler names:
exception_trampoline supervisor_call_trampoline, 0, software_interrupt_c
exception_trampoline undefined_instruction_trampoline, 0, undefined_instruction_c
exception_trampoline prefetch_abort_trampoline, 0, prefetch_abort_c  
exception_trampoline data_abort_trampoline, 0, data_abort_c
exception_trampoline not_used_trampoline, 0, not_used_c
exception_trampoline irq_trampoline, 4, irq_c
exception_trampoline fiq_trampoline, 4, fiq_c
