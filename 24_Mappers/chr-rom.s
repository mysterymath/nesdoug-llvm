.globl __chr_rom_size
__chr_rom_size = 32

.section .chr_rom_0,"a",@progbits
.incbin "apples.chr"

.section .chr_rom_1,"a",@progbits
.incbin "balls.chr"

.section .chr_rom_2,"a",@progbits
.incbin "snake.chr"

.section .chr_rom_3,"a",@progbits
.incbin "flower.chr"
