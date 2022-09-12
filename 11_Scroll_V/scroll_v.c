/*	example code for llvm-mos, for NES
 *  scrolling, with horizontal mirroring (vertical arrangement)
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>

#include "BG/N.h"    // backgrounds compressed as rle files
#include "Sprites.h" // holds our metasprite data

// Horizontal mirroring (the default).
asm(".globl __mirroring\n__mirroring = 0\n");

// PROTOTYPES
void draw_sprites(void);
void scrolling(char pad1);

int main(void) {

  ppu_off(); // screen off

  // load the palettes
  static const unsigned char palette_bg[16] = {
      0x0f, 0x00, 0x10, 0x30, // black, dk gray, lt gray, white
  };
  pal_bg(palette_bg);
  static const unsigned char palette_sp[16] = {
      0x0f, 0x0f, 0x0f, 0x28, // black, black, yellow
      0x0f, 0x0f, 0x0f, 0x15, // black, black, pink
  };
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprites are set to 0 by default
  bank_spr(1);

  vram_adr(NAMETABLE_A);
  // this sets a start position on the BG, top left of screen A

  vram_unrle(N0);
  // this unpacks a compressed full nametable

  vram_adr(NAMETABLE_C);
  // this sets a start position on the BG, top left of screen B

  vram_unrle(N2);
  // this unpacks a compressed full nametable

  ppu_on_all(); // turn on screen

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame
    // the sprites are pushed from a buffer to the OAM during nmi

    char pad1 = pad_poll(0); // read the first controller

    scrolling(pad1);
    draw_sprites();
  }
}

static unsigned scroll_x, scroll_y;

void draw_sprites(void) {
  // clear all sprites from sprite buffer
  oam_clear();

  // draw 1 metasprite
  const struct { char x, y; } sprite = {0x80, 0x80};
  oam_meta_spr(sprite.x, sprite.y, YellowSpr);

  // draw the x and y as sprites
  oam_spr(20, 20, 0xfe, 1); // 0xfe = X
  // oam_spr(unsigned char x,unsigned char y,unsigned char chrnum,unsigned char
  // attr);
  oam_spr(28, 20, (scroll_x & 0xff) >> 4, 1);
  oam_spr(36, 20, (scroll_x & 0x0f), 1);
  oam_spr(50, 20, 0xff, 1); // 0xff = Y
  oam_spr(58, 20, (scroll_y & 0xff) >> 4, 1);
  oam_spr(66, 20, (scroll_y & 0x0f), 1);
}

void scrolling(char pad1) {
  if (pad1 & PAD_LEFT) {
    scroll_x -= 1;
  } else if (pad1 & PAD_RIGHT) {
    scroll_x += 1;
  }
  if (pad1 & PAD_UP) {
    scroll_y = sub_scroll_y(1, scroll_y);
  } else if (pad1 & PAD_DOWN) {
    scroll_y = add_scroll_y(1, scroll_y);
  }
  // scroll y is tricky, since values 240-255 are treated as -16 to -1
  // add_scroll_y,sub_scroll_y will adjust 16 if cross this range

  // set scroll
  set_scroll_x(scroll_x);
  set_scroll_y(scroll_y);
}
