OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
BASE_ADDRESS = 0x400;
LOADER_SEG_PHYADDR = 0x90000;

SECTIONS
{
    . = BASE_ADDRESS;
    .text.s16 : {
        build/loader/loader.asm.obj(.text.s16)
    }

    . = ALIGN(32);
    . += LOADER_SEG_PHYADDR;
    text_lma = . - LOADER_SEG_PHYADDR;
    .text ALIGN(32): AT(text_lma) {
        *(.text .stub .text.* .gnu.linkonce.t.* .rodata .rodata.*)
    }

    . = ALIGN(32);
    data_lma = . - LOADER_SEG_PHYADDR;
    .data ALIGN(32): AT(data_lma) {
        *(*)
    }

    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack)
    }
}
