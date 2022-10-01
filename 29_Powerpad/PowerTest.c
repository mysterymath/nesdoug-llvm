/*	example code for cc65, for NES
 *  test power pad
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>
#include <padlib.h>

#include "NES_ST/PowerBG.h"

unsigned powerpad_cur;
unsigned powerpad_old; // don't use this
unsigned powerpad_new;

const char pal1[16] = {
    // clang-format off
    0x0f, 0x11, 0x16, 0x30,
    0x0f, 0x00, 0x10, 0x30,
    // clang-format on
};

const char pal2[16] = {
    // clang-format off
    0x0f, 0x0f, 0x0f, 0x0f, // black
    // clang-format on
};

// do after the read
void process_powerpad(void) {
  powerpad_new = (powerpad_cur ^ powerpad_old) & powerpad_cur;
  powerpad_old = powerpad_cur;
}

int main(void) {
  ppu_off(); // screen off

  // load the palettes
  pal_bg(pal1);
  pal_spr(pal2);

  // use the second set of tiles for sprites
  bank_spr(1);

  vram_adr(NAMETABLE_A); // set a start position for the text

  vram_unrle(PowerBG);

  ppu_on_all(); // turn on screen

  while (1) {
    // infinite loop

    ppu_wait_nmi(); // wait till beginning of the frame

    oam_clear();

    powerpad_cur = read_powerpad(1);
    process_powerpad(); // goes after the read
                        // transfers only new presses to powerpad_new
                        // powerpad_new isn't used here, but
                        // would be very useful for a game

    if (powerpad_cur & POWERPAD_1)
      oam_spr(84, 83, 0, 0);
    if (powerpad_cur & POWERPAD_2)
      oam_spr(100, 83, 0, 0);
    if (powerpad_cur & POWERPAD_3)
      oam_spr(116, 83, 0, 0);
    if (powerpad_cur & POWERPAD_4)
      oam_spr(132, 83, 0, 0);
    if (powerpad_cur & POWERPAD_5)
      oam_spr(84, 107, 0, 0);
    if (powerpad_cur & POWERPAD_6)
      oam_spr(100, 107, 0, 0);
    if (powerpad_cur & POWERPAD_7)
      oam_spr(116, 107, 0, 0);
    if (powerpad_cur & POWERPAD_8)
      oam_spr(132, 107, 0, 0);
    if (powerpad_cur & POWERPAD_9)
      oam_spr(84, 131, 0, 0);
    if (powerpad_cur & POWERPAD_10)
      oam_spr(100, 131, 0, 0);
    if (powerpad_cur & POWERPAD_11)
      oam_spr(116, 131, 0, 0);
    if (powerpad_cur & POWERPAD_12)
      oam_spr(132, 131, 0, 0);
  }
}
