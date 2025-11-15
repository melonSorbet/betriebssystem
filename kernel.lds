RAM_BASE = 0x00000000;
RAM_SIZE = 0x40000000;
STACK_REGION_SIZE = 0x5000;
ENTRY(_start)
SECTIONS
{
	. = 0x00000000;
    	.ivt : { *(.ivt) }

	. = 0x00008000;
	.init : { *(.init) }
	.text : { *(.text) }
	.rodata : { *(.rodata) }
	. = ALIGN(1<<20);
	.data : { *(.data) }
	.bbs  : { *(.bbs)  }
	. = RAM_BASE + RAM_SIZE - STACK_REGION_SIZE;
	. = ALIGN(8);
	__stack_region_start = .;

	/* IRQ stack */
	. += 0x1000;
	irq_stack_top = .;

	/* SVC stack */
	. += 0x1000;
	svc_stack_top = .;

	/* Abort stack */
	. += 0x1000;
	abort_stack_top = .;

	/* Undefined stack */
	. += 0x1000;
	und_stack_top = .;

	/* System/User stack */
	. += 0x1000;
	sys_stack_top = .;

	__stack_region_end = .;
}
