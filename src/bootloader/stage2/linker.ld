ENTRY(entry)
OUTPUT_FORMAT("binary")
phys_addr = 0x00000500;

SECTIONS
{
  . = phys_addr;

  .entry                : { __entry_start = .;  *(.entry) }
  .text                 : { __text_start = .;   *(.text) }
  .data                 : { __data_start = .;   *(.data) }
  .rodata               : { __rodata_start = .; *(.rodata) }
  .bss                  : { __bss_start = .;    *(.bss) }

  __end = .;
}
