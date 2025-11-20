
#include <stdint.h>

struct mode_regs {
    uint32_t user_sp;
    uint32_t user_lr;

    uint32_t irq_sp;
    uint32_t irq_lr;

    uint32_t abort_sp;
    uint32_t abort_lr;

    uint32_t undefined_sp;
    uint32_t undefined_lr;

    uint32_t supervisor_sp;
    uint32_t supervisor_lr;
};

static inline struct mode_regs read_mode_specific_registers(void) {
    struct mode_regs regs;

    // User/System mode
    asm volatile(
        "mrs %0, cpsr\n\t"
        "mov %1, sp\n\t"
        "mov %2, lr\n\t"
        : "=r"(regs.user_sp), "=r"(regs.user_sp), "=r"(regs.user_lr)
    );

    // IRQ mode
    asm volatile(
        "cps #0x12\n\t"   // Switch to IRQ
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        "cps #0x13\n\t"   // Switch back to SVC
        : "=r"(regs.irq_sp), "=r"(regs.irq_lr)
    );

    // Abort mode
    asm volatile(
        "cps #0x17\n\t"
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        "cps #0x13\n\t"
        : "=r"(regs.abort_sp), "=r"(regs.abort_lr)
    );

    // Undefined mode
    asm volatile(
        "cps #0x1B\n\t"
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        "cps #0x13\n\t"
        : "=r"(regs.undefined_sp), "=r"(regs.undefined_lr)
    );

    // Supervisor mode
    asm volatile(
        "cps #0x13\n\t"
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        : "=r"(regs.supervisor_sp), "=r"(regs.supervisor_lr)
    );

    return regs;
}


#include <stdint.h>

/* Data Fault Status Register (DFSR) */
static inline unsigned int read_dfsr(void) {
    unsigned int dfsr;
    __asm__ volatile ("mrc p15, 0, %0, c5, c0, 0" : "=r" (dfsr));
    return dfsr;
}

/* Data Fault Address Register (DFAR) */
static inline unsigned int read_dfar(void) {
    unsigned int dfar;
    __asm__ volatile ("mrc p15, 0, %0, c6, c0, 0" : "=r" (dfar));
    return dfar;
}

/* Instruction Fault Status Register (IFSR) */
static inline unsigned int read_ifsr(void) {
    unsigned int ifsr;
    __asm__ volatile ("mrc p15, 0, %0, c5, c0, 1" : "=r" (ifsr));
    return ifsr;
}

/* Instruction Fault Address Register (IFAR) */
static inline unsigned int read_ifar(void) {
    unsigned int ifar;
    __asm__ volatile ("mrc p15, 0, %0, c6, c0, 2" : "=r" (ifar));
    return ifar;
}
