OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
MEMORY { .sram : ORIGIN = 0x0,LENGTH = 32k }
ENTRY(main)
SECTIONS {
    .text :{
        src/spl.o	(.text)
        *(.text)
    } >.sram
    . = ALIGN(4);
    .data : { *(.data) } >.sram
    . = ALIGN(4);
    .bss : { *(.bss) } >.sram
}