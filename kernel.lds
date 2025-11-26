STACK_SIZE = 0x800;    /* 2 KiB stack (changed from 4 KiB) */
ENTRY(_start)
SECTIONS
{
	. = 0x00008000;
	.init : { *(.init) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	. = ALIGN(1<<20);
	.data : { *(.data) }
	.bss  : { *(.bss)  }
	
	/* Stack definitions - 2 KiB (0x800) per stack, descending order */
	/* SVC stack: 0x100000 - 0xFF800 */
	. = 0x100000;
	svc_stack_top = .;
	svc_stack_bottom = . - STACK_SIZE;
	
	/* UND stack: 0xFF800 - 0xFF000 */
	. = 0xFF800;
	und_stack_top = .;
	und_stack_bottom = . - STACK_SIZE;
	
	/* ABT stack: 0xFF000 - 0xFE800 */
	. = 0xFF000;
	abort_stack_top = .;
	abort_stack_bottom = . - STACK_SIZE;
	
	/* IRQ stack: 0xFE800 - 0xFE000 */
	. = 0xFE800;
	irq_stack_top = .;
	irq_stack_bottom = . - STACK_SIZE;
	
	/* SYS stack: 0xFE000 - 0xFD800 */
	. = 0xFE000;
	sys_stack_top = .;
	sys_stack_bottom = . - STACK_SIZE;
	
	/* FIQ stack: 0xFD800 - 0xFD000 */
	. = 0xFD800;
	fiq_stack_top = .;
	fiq_stack_bottom = . - STACK_SIZE;
	
	__stack_start = 0xFD000;  /* Start of first stack (FIQ) */
	__stack_end = 0x100000;   /* End of last stack (SVC) */
}
