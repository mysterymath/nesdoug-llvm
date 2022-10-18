SECTIONS {
    .ft2 : {
        *libfamitone2.a:*(.text .text.* .rodata .rodata.*)
    } >prg_rom_6_vma AT>prg_rom_6_lma
}
INCLUDE link.ld
