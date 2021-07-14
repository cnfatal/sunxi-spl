MEMORY { .sram : ORIGIN = 0x0,LENGTH = 32k }
ENTRY(main)
SECTIONS {
	. = 0x0000;
	.start : { *(.start) } >.sram
    .text :{
        *(.text)
    } >.sram
    . = ALIGN(4);
    .data : { *(.data) } >.sram
    . = ALIGN(4);
    .bss : { *(.bss) } >.sram
}