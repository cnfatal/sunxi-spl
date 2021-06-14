;MEMORY { .sram : ORIGIN = 0x2000,LENGTH = 32k }
ENTRY(main)
SECTIONS {
    .text :{
        fes1_head.o	(.text)
        *(.text)
    } >.sram
    .data : { *(.data) } >.sram
    .bss : { *(.bss) } >.sram
}