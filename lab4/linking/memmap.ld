SECTIONS
{
    .text 0x40000000      :  { *(.text.start) *(.text*) }
    __text_end            = .;
    .rodata               :  { *(.rodata*) }
    .data ALIGN(0x20000)  :  { *(.sdata*) }
    __bss_start           = .;
    .bss                  :  { *(.bss*)  *(COMMON) }
    __bss_end             = .;
}

ENTRY(_start)
