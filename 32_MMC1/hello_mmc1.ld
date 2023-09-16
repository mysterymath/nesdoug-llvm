SECTIONS {
    .ft2 : {
        *libfamitone2.a:*(.text .text.* .rodata .rodata.*)
    } >prg_rom_6
}
INCLUDE link.ld
