OUTPUT_ARCH( "riscv" )
OUTPUT_FORMAT("elf64-littleriscv")
ENTRY( _start )
SECTIONS
{
  /* text: test code section */
  . = 0x80000000;
  .text : { *(.text) }
  /* data: Initialized data segment */
  .gnu_build_id : { *(.note.gnu.build-id) }
  .data : { *(.data) }
  .rodata : { *(.rodata) }
  .sdata : { *(.sdata) }
  .debug : { *(.debug) }
  . += 0x8000;
  . = ALIGN(16);
  stack_top = .;

  /* Heap */
  . = ALIGN(8);
  PROVIDE(heap_bottom = .);
  . += 0x800000;

  /* Pages */
  . = ALIGN(4096);
  PROVIDE(pages_bottom = .);

  /* End of uninitalized data segement */
}

