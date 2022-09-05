/*	example code for llvm-mos, for NES
 *  test fading in and out (w full screen image)
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>

#include "NES_ST/Girl5.h"

int main(void) {
  static const char palette[16] = {0x08, 0x16, 0x26, 0x38, 0x08, 0x0f,
                                   0x0f, 0x0f, 0x08, 0x0f, 0x0f, 0x0f,
                                   0x08, 0x0f, 0x0f, 0x0f};

  ppu_off(); // screen off

  pal_bg(palette); //	load the palette

  vram_adr(NAMETABLE_A);
  // this sets a start position on the BG, top left of screen

  vram_unrle(Girl5);
  // this unpacks a compressed full nametable

  pal_bright(0); // can be a value 0 (black) to 8 (white), 4 = normal
                 // the init code set it to 4 by default

  ppu_on_all(); // turn on screen

  delay(10); // wait 10 frames

  while (1) {

    pal_fade_to(0, 4); // (from, to) fade in to normal

    delay(100); // wait 100 frames

    pal_fade_to(4, 0); // (from, to) fade to black

    delay(100); // wait 100 frames
  }
}
