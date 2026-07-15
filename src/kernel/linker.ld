OUTPUT_FORMAT("elf64-x86-64")
ENTRY(kmain)

SECTIONS
{
  /* kernel is loaded to virtual address 0xFFFFFFFF80100000 */
  . = 0xFFFFFFFF80100000;

  .text : {
    *(.entry)
    *(.text)
  }

  .data : {
    *(.data)
  }

  .rodata : {
    *(.rodata)
  }

  .bss : {
    *(.bss)
  }
}
