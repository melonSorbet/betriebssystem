STACK_SIZE = 0x800;
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

    . = 0x100000;
    svc_stack_top = .;
    svc_stack_bottom = . - STACK_SIZE;

    . = 0xFF800;
    und_stack_top = .;
    und_stack_bottom = . - STACK_SIZE;

    . = 0xFF000;
    abort_stack_top = .;
    abort_stack_bottom = . - STACK_SIZE;

    . = 0xFE800;
    irq_stack_top = .;
    irq_stack_bottom = . - STACK_SIZE;

    . = 0xFE000;
    sys_stack_top = .;
    sys_stack_bottom = . - STACK_SIZE;

    . = 0xFD800;
    fiq_stack_top = .;
    fiq_stack_bottom = . - STACK_SIZE;

    __stack_start = 0xFD000;
    __stack_end = 0x100000;
}

