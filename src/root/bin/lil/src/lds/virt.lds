OUTPUT_ARCH( "riscv" )
OUTPUT_FORMAT( "elf64-littleriscv" )
ENTRY( _start )
SECTIONS
{
    . = 0x40000;

    /* data: Initialized data segment */
    .bss : ALIGN(4096) { *(.bss) }
    .gnu_build_id : ALIGN(4096) { *(.note.gnu.build-id) }
    .data : ALIGN(4096) { *(.data) }
    .rodata : ALIGN(4096) { *(.rodata) }
    .sdata : ALIGN(4096) { *(.sdata) }
    .debug : ALIGN(4096) { *(.debug) }
    /* End of uninitalized data segement */

    /* text: text code section */
    .text : ALIGN(4096) { *(.text) }
}

