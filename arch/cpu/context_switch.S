.section .text

.global context_switch
context_switch:
    // Save old context
    push {r4-r11, lr}
    mov r2, sp
    str r2, [r0, #40]  // Save SP
    
    pop {r4-r11, lr}   // Restore to continue saving
    stmia r0!, {r0-r12, lr}
    
    // Save return address from exception stack
    mrs r2, spsr
    str r2, [r0], #4
    
    // Load new context
    ldmia r1!, {r0-r12, lr}
    
    // Restore stack pointer
    ldr sp, [r1], #4
    
    // Restore CPSR and return
    ldr r2, [r1]
    msr spsr_cxsf, r2
    
    // Return from exception
    movs pc, lr

.global init_user_context
init_user_context:
    // r0 = context pointer
    // r1 = function pointer
    // r2 = stack top
    // r3 = argument pointer
    
    // Initialize all registers to 0
    mov r4, #0
    mov r5, #0
    mov r6, #0
    mov r7, #0
    mov r8, #0
    mov r9, #0
    mov r10, #0
    mov r11, #0
    mov r12, #0
    
    stmia r0!, {r4-r12}  // Save r0-r12 as 0
    
    // Set argument in r0
    str r3, [r0, #-52]   // r0 at offset 0
    
    // Set stack pointer
    mov r4, r2
    str r4, [r0], #4     // sp
    
    // Set lr to thread_exit
    ldr r4, =thread_exit
    str r4, [r0], #4     // lr
    
    // Set pc to function
    str r1, [r0], #4     // pc
    
    // Set CPSR to user mode
    mov r4, #0x10
    str r4, [r0]         // cpsr
    
    bx lr

.global thread_exit
thread_exit:
    svc #0
    b .
