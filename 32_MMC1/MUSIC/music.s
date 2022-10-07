.section .prg_rom_6.music_data,"a",@progbits
.globl music_data
music_data:
  .include "TestMusic3.s"

.section .prg_rom_6.sounds_data,"a",@progbits
.globl sounds_data
sounds_data:
  .include "SoundFx.s"

.section .dpcm,"a",@progbits
  .incbin "BassDrum.dmc"
