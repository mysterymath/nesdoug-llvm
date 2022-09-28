/*	simple Hello World, for llvm-mos, for NES
 *  switching BG tileset with mapper CNROM
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>

#include "NES_ST/all_bgs.h"

const char pal_apple[16] = {0x08, 0x16, 0x27, 0x38};
const char pal_ball[16] = {0x06, 0x27, 0x12, 0x16};
const char pal_snake[16] = {0x0f, 0x17, 0x29, 0x39};
const char pal_flower[16] = {0x08, 0x19, 0x27, 0x30};

// Place this explicitly in rodata to keep it from being lifted to ZP.
__attribute__((section(".rodata.bus_conflict_fix")))
static const char bus_conflict_fix[] = {0, 1, 2, 3};

int main(void) {
  ppu_off(); // screen off

  pal_bg(pal_apple); // load the palette
  POKE(&bus_conflict_fix[0], 0);

  vram_adr(NAMETABLE_A);

  vram_unrle(all_bgs);

  ppu_on_all(); // turn on screen

  char which_chr = 0;
  while (1) {
    ppu_wait_nmi();

    char pad1 = pad_poll(0);
    char pad1_new = get_pad_new(0);

    if (pad1_new & PAD_START) {
      if (++which_chr >= 4)
        which_chr = 0;

      static const char *const palettes[] = {pal_apple, pal_ball, pal_snake,
                                             pal_flower};
      pal_bg(palettes[which_chr]); // change the bg palette
      POKE(&bus_conflict_fix[which_chr], which_chr); // change the tileset
    }
  }
}
