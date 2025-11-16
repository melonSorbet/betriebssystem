RAM_BASE = 0x00000000;
RAM_SIZE = 0x40000000;
STACK_REGION_SIZE = 0x5000;

STACK_SIZE = 0x1000;   /* 4 KiB stack */
STACK_GAP  = 0x400;    /* 1 KiB gap between stacks */
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
	. = ALIGN(8); /* Ensure 8-byte alignment */
	__stack_start = .;

	/* Reserve space for each stack */
	. += 0x1000; /* 4KB for IRQ stack */
	irq_stack_top = .;

	. += 0x1000; /* 4KB for SVC stack */
	svc_stack_top = .;

	. += 0x1000; /* 4KB for Abort stack */
	abort_stack_top = .;

	. += 0x1000; /* 4KB for Undefined stack */
	und_stack_top = .;

	. += 0x1000; /* 4KB for System stack */
	sys_stack_top = .;

	. += 0x1000; /* 4KB for FIQ stack */
	fiq_stack_top = .;

	__stack_end = .;
    }
