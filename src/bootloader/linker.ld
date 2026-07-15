OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

SECTIONS
{
  /* kernel expects to be loaded to address 0x7E00 */
  . = 0x7E00;

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
