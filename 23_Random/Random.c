/*	example code for cc65, for NES
 *  construct some sprites - put 64 sprites in random positions
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>

#include "Sprites.h" // holds our metasprite data

int main(void) {

  ppu_off(); // screen off

  // load the palettes
  static const char palette_bg[16] = {
      0x0f, 0x00, 0x10, 0x30, // black, gray, lt gray, white
  };
  pal_bg(palette_bg);

  static const char palette_sp[16] = {
      0x0f, 0x0f, 0x0f, 0x28, // black, black, yellow
  };
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprite are set to 0 by default
  bank_spr(1);

  // load the text
  // vram_adr(NTADR_A(x,y));
  vram_adr(NTADR_A(7, 14)); // set a start position for the text

  // vram_write draws the array to the screen
  static const char text[] = "Press Start";
  vram_write(text, sizeof(text));

  ppu_on_all(); // turn on screen

  char spr_x[64];
  char spr_y[64];
  char start_pressed = 0;

  while (1) {
    // infinite loop

    ppu_wait_nmi(); // wait till beginning of the frame
    // the sprites are pushed from a buffer to the OAM during nmi

    oam_clear();

    char pad1 = pad_poll(0); // read the first controller
    if (!start_pressed) {
      if (pad1 & PAD_START) {
        ++start_pressed;
        seed_rng(); // the frame count was ticking up every frame till Start
                    // pressed
        for (char i = 0; i < 64; ++i) {
          spr_x[i] = rand8(); // fill the arrays with random #s
          spr_y[i] = rand8();
        }
      }
    } else { // fill the sprite buffer
      char i = 0;
      for (; i < 25; ++i) {
        if (get_frame_count() & 1) { // half the time
          spr_y[i] = spr_y[i] + 1;   // fall
        }
        oam_spr(spr_x[i], spr_y[i], 0, 0);
      }
      for (; i < 55; ++i) {
        spr_y[i] = spr_y[i] + 1; // fall
        oam_spr(spr_x[i], spr_y[i], 0, 0);
      }
      for (; i < 64; ++i) {
        spr_y[i] = spr_y[i] + 2; // fall fast
        oam_spr(spr_x[i], spr_y[i], 0, 0);
      }
    }
  }
}
