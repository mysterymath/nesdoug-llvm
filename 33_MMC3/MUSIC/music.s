.section .prg_rom_12.music_data,"a",@progbits
.globl music_data
music_data:
  .include "TestMusic3.s"

.section .prg_rom_12.sounds_data,"a",@progbits
.globl sounds_data
sounds_data:
  .include "SoundFx.s"
