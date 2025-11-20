#include <arch/cpu/interrupts.h>
#include <lib/print_exception.h>
#include <lib/kprintf.h>
#include <stddef.h>
#include <arch/cpu/mode_registers.h>
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
){

	kprintf("############ EXCEPTION ############\n");
	kprintf("%s an Adresse: 0x%08x\n", exception_name, exception_source_addr);
	if(is_data_abort) {
		const char* dfsr_description = get_fsr_description(data_fault_status_register);
		kprintf("Data Fault Status Register: 0x%08x -> %s\n", data_fault_status_register, dfsr_description);
		kprintf("Data Fault Adress Register: 0x%08x\n", data_fault_address_register);
	}
		
	if(is_prefetch_abort) {
		const char* ifsr_description = get_fsr_description(instruction_fault_status_register);
		kprintf("Instruction Fault Status Register: 0x%08x -> %s\n", instruction_fault_status_register, ifsr_description);
		kprintf("Instruction Fault Adress Register: 0x%08x\n", instruction_fault_address_register);
	}
			
	kprintf("\n>> Registerschnappschuss <<\n");
	kprintf("R0: 0x%08x  R5: 0x%08x  R10: 0x%08x\n", frame->r0, frame->r5, frame->r10);
	kprintf("R1: 0x%08x  R6: 0x%08x  R11: 0x%08x\n", frame->r1, frame->r6, frame->r11);
	kprintf("R2: 0x%08x  R7: 0x%08x  R12: 0x%08x\n", frame->r2, frame->r7, frame->r12);
	kprintf("R3: 0x%08x  R8: 0x%08x\n", frame->r3, frame->r8);
	kprintf("R4: 0x%08x  R9: 0x%08x\n", frame->r4, frame->r9);

	struct mode_regs mode_regs = read_mode_specific_registers();

	kprintf("\n>> Modusspezifische Register <<\n");
	kprintf("User/System | LR: 0x%08x | SP: 0x%08x | CPSR: ", mode_regs.user_lr, mode_regs.user_sp);
	print_psr(cpsr);
	kprintf("\nIRQ         | LR: 0x%08x | SP: 0x%08x | SPSR: ", mode_regs.irq_lr, mode_regs.irq_sp);
	print_psr(irq_spsr);
	kprintf("\nAbort       | LR: 0x%08x | SP: 0x%08x | SPSR: ", mode_regs.abort_lr, mode_regs.abort_sp);
	print_psr(abort_spsr);
	kprintf("\nUndefined   | LR: 0x%08x | SP: 0x%08x | SPSR: ", mode_regs.undefined_lr, mode_regs.undefined_sp);
	print_psr(undefined_spsr);
	kprintf("\nSupervisor  | LR: 0x%08x | SP: 0x%08x | SPSR: ", mode_regs.supervisor_lr, mode_regs.supervisor_sp);
	print_psr(supervisor_spsr);
	kprintf("\n");
}

static const char* get_mode_name(unsigned int cpsr) {
    switch(cpsr & 0x1F) {  // mask for mode bits
        case 0x10: return "User";
        case 0x11: return "FIQ";
        case 0x12: return "IRQ";
        case 0x13: return "SVC";
        case 0x17: return "Abort";
        case 0x1B: return "Undefined";
        case 0x1F: return "System";
        default:   return "Unknown";
    }
}

void print_psr(unsigned int psr) {
    // extract N,Z,C,V,E,I,F,T bits
    unsigned char psr_n = (psr >> 31) & 1;
    unsigned char psr_z = (psr >> 30) & 1;
    unsigned char psr_c = (psr >> 29) & 1;
    unsigned char psr_v = (psr >> 28) & 1;
    unsigned char psr_e = (psr >> 9) & 1;
    unsigned char psr_i = (psr >> 7) & 1;
    unsigned char psr_f = (psr >> 6) & 1;
    unsigned char psr_t = (psr >> 5) & 1;

    const char* mode_name = get_mode_name(psr);

    kprintf("%c%c%c%c %c %c%c%c", 
        psr_n ? 'N' : '_',
        psr_z ? 'Z' : '_',
        psr_c ? 'C' : '_',
        psr_v ? 'V' : '_',
        psr_e ? 'E' : '_',
        psr_i ? 'I' : '_',
        psr_f ? 'F' : '_',
        psr_t ? 'T' : '_'
    );

    kprintf(" %s 0x%08x", mode_name, psr);
}


const char* get_fsr_description(unsigned int fsr){
    static const char *fsr_sources[] = {
        [0b00000] =  "No function, reset value",
        [0b00001] =  "Alignment fault",
        [0b00010] =  "Debug event fault",
        [0b00011] =  "Access Flag fault on Section",
        [0b00100] =  "Cache maintenance operation fault",
        [0b00101] =  "Translation fault on Section",
        [0b00110] =  "Access Flag fault on Page",
        [0b00111] =  "Translation fault on Page",
        [0b01000] =  "Precise External Abort",
        [0b01001] =  "Domain fault on Section",
        [0b01011] =  "Domain fault on Page",
        [0b01100] =  "External abort on Section",
        [0b01101] =  "Permission fault on Section",
        [0b01110] =  "External abort on Page",
        [0b01111] =  "Permission fault on Page",
        [0b10000] =  "TLB conflict abort",
        [0b10100] =  "Implementation definedf fault",
        [0b10110] =  "External Abort",
        [0b11000] =  "Asynchronous parity error on memory access",
        [0b11001] =  "Synchronous parity error on memory access",
        [0b11010] =  "Implementation defined fault",
        [0b11100] =  "Synchronous parity error on translation table walk on section",
        [0b11110] =  "Synchronous parity error on translation table walk on page",
    };

    constexpr int fsr_status_4_index = 10;
    unsigned int fsr_status = (fsr & 0b1111) |
     ((fsr & (1 << (fsr_status_4_index - 1))) >> (fsr_status_4_index - 4 - 1));

    if(fsr_status > sizeof(fsr_sources) / sizeof(const char*) ||
       fsr_sources[fsr_status] == NULL){
        return "Invalid fault status register value";
    }

    return fsr_sources[fsr_status];
}
