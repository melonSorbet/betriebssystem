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
	. = RAM_BASE + RAM_SIZE - (STACK_SIZE+STACK_GAP)*6; /* adjust total stacks+gaps */
	. = ALIGN(8);
	__stack_region_start = .;

	fiq_stack_top   = . + STACK_SIZE;
	. += STACK_SIZE + STACK_GAP;

	irq_stack_top   = . + STACK_SIZE;
	. += STACK_SIZE + STACK_GAP;

	abort_stack_top = . + STACK_SIZE;
	. += STACK_SIZE + STACK_GAP;

	und_stack_top   = . + STACK_SIZE;
	. += STACK_SIZE + STACK_GAP;

	svc_stack_top   = . + STACK_SIZE;
	. += STACK_SIZE + STACK_GAP;

	sys_stack_top   = . + STACK_SIZE;
	. += STACK_SIZE + STACK_GAP;

}
