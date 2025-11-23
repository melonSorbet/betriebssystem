
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
    struct mode_regs regs = {0};
    unsigned int sp, lr;

    // Supervisor mode
    asm volatile(
        "cps #0x13\n\t"     // Switch to SVC
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        : "=r"(sp), "=r"(lr)
    );
    regs.supervisor_sp = sp;
    regs.supervisor_lr = lr;

    // IRQ mode
    asm volatile(
        "cps #0x12\n\t"
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        : "=r"(sp), "=r"(lr)
    );
    regs.irq_sp = sp;
    regs.irq_lr = lr;

    // Abort mode
    asm volatile(
        "cps #0x17\n\t"
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        : "=r"(sp), "=r"(lr)
    );
    regs.abort_sp = sp;
    regs.abort_lr = lr;

    // Undefined mode
    asm volatile(
        "cps #0x1B\n\t"
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        : "=r"(sp), "=r"(lr)
    );
    regs.undefined_sp = sp;
    regs.undefined_lr = lr;

    // User/System mode
    asm volatile(
        "cps #0x1F\n\t"
        "mov %0, sp\n\t"
        "mov %1, lr\n\t"
        : "=r"(sp), "=r"(lr)
    );
    regs.user_sp = sp;
    regs.user_lr = lr;

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
