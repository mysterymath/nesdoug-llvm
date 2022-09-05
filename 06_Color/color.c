/*	example code for llvm-mos, for NES
 *  test the attribute table
 *  -writing to PPU with rendering off
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>

#include "NES_ST/blocks.h"

int main(void) {
  static const char palette[] = {0x0f, 0x00, 0x10, 0x30,  // grays
                                 0x0f, 0x01, 0x21, 0x31,  // blues
                                 0x0f, 0x06, 0x26, 0x36,  // reds
                                 0x0f, 0x09, 0x29, 0x39}; // greens

  ppu_off(); // screen off

  pal_bg(palette); //	load the palette

  vram_adr(NAMETABLE_A);
  // this sets a start position on the BG, top left of screen

  vram_unrle(blocks);
  // this unpacks a compressed full nametable

  // get_at_addr(char nt, char x, char y); // pixel positions 0-0xff
  int address = get_at_addr(0, 0, 0);
  vram_adr(address); // nametable A's attribute table 23c0-23ff

  vram_fill(0, 8);    // 8 bytes of 00 00 00 00
  vram_fill(0x55, 8); // 8 bytes of 01 01 01 01
  vram_fill(0xAA, 8); // 8 bytes of 10 10 10 10
  vram_fill(0xFF, 8); // 8 bytes of 11 11 11 11

  address = get_at_addr(0, 0x40, 0xa0);
  vram_adr(address); // pick a random attribute byte on the lower half

  vram_put(0xe4); // push 1 byte 11 10 01 00

  // note they go on the screen like this
  // 00  01
  // 10  11

  ppu_on_all(); // turn on screen

  while (1) {
    // infinite loop
    // game code can go here later.
  }
}
