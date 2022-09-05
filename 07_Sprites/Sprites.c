/*	example code for llvm-mos, for NES
 *  construct some sprites / metasprites
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>

#include "Sprites.h"

int main(void) {
  static const char text[] = "Sprites";

  static const char palette_bg[16] = {
      0x0f, 0x00, 0x10, 0x30 // black, gray, lt gray, white
  };

  static const char palette_sp[16] = {
      0x0f, 0x0f, 0x0f, 0x28 // black, black, yellow
  };

  ppu_off(); // screen off

  // load the palettes
  pal_bg(palette_bg);
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprite are set to 0 by default
  bank_spr(1);

  // load the text
  // vram_adr(NTADR_A(x,y));
  vram_adr(NTADR_A(7, 14)); // set a start position for the text

  // vram_write draws the array to the screen
  vram_write(text, sizeof(text));

  ppu_on_all(); // turn on screen

  char y_position = 0x40; // all share same Y, which increases every frame
  char x_position = 0x88;
  char x_position2 = 0xa0;
  char x_position3 = 0xc0;
  while (1) {
    // infinite loop

    ppu_wait_nmi(); // wait till beginning of the frame
    // the sprites are pushed from a buffer to the OAM during nmi

    // clear all sprites from sprite buffer
    oam_clear();

    // push a single sprite
    // oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned
    // char attr); use tile #0, palette #0
    oam_spr(x_position, y_position, 0, 0);

    // push a metasprite
    // oam_meta_spr(unsigned char x,unsigned char y,const unsigned char *data);
    oam_meta_spr(x_position2, y_position, metasprite);

    // and another
    oam_meta_spr(x_position3, y_position, metasprite2);

    ++y_position; // every frame, shift them down 1 pixel
                  // note, y positions 0xef-0xff will not appear on the screen
  }
}
