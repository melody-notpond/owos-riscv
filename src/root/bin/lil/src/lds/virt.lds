OUTPUT_ARCH( "riscv" )
OUTPUT_FORMAT( "elf64-littleriscv" )
ENTRY( _start )
SECTIONS
{
    /* text: text code section */
    . = 0x40000;
    .text : { *(.text) }

    /* data: Initialized data segment */
    .gnu_build_id : { *(.note.gnu.build-id) }
    .data : { *(.data) }
    .rodata : { *(.rodata) }
    .sdata : { *(.sdata) }
    .debug : { *(.debug) }
    /* End of uninitalized data segement */
}

