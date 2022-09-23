.section .rodata.music_data,"a",@progbits
.globl music_data
music_data:
  .include "TestMusic3.s"

.section .rodata.sounds_data,"a",@progbits
.globl sounds_data
sounds_data:
  .include "SoundFx.s"
