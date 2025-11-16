#include <arch/cpu/interrupts.h>

void print_psr(unsigned int psr);

const char* get_fsr_description(unsigned int fsr);

void print_exception_infos(
    exc_frame_t* frame,
    const char* exception_name,
    unsigned int exception_source_addr,
    bool is_data_abort,
    bool is_prefetch_abort,
    unsigned int data_fault_status_register,  // Data Fault Status Register
    unsigned int data_fault_address_register,  // Data Fault Address Register
    unsigned int instruction_fault_status_register,  // Instruction Fault Status Register
    unsigned int instruction_fault_address_register,  // Instruction Fault Address Register
    unsigned int cpsr,
    unsigned int irq_spsr,
    unsigned int abort_spsr,
    unsigned int undefined_spsr,
    unsigned int supervisor_spsr
);
