.section .rodata.music_data,"a",@progbits
.globl music_data
music_data:
  .incbin "TestMusic3.s"

.section .rodata.sounds_data,"a",@progbits
.globl sounds_data
sounds_data:
  .incbin "SoundFx.s"
