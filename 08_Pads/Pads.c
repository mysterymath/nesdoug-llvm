/*	example code for cc65, for NES
 *  move some sprites with the controllers
 *  -also sprite vs sprite collisions
 *	using neslib
 *	Doug Fraker 2018
 */

#include "Sprites.h"

#include <nesdoug.h>
#include <neslib.h>

struct BoxGuy {
  char x;
  char y;
  char width;
  char height;
};

struct BoxGuy BoxGuy1 = {20, 20, 15, 15};
struct BoxGuy BoxGuy2 = {70, 20, 15, 15};
// the width and height should be 1 less than the dimensions (16x16)
// note, I'm using the top left as the 0,0 on x,y

// PROTOTYPES
void draw_sprites(void);
void movement(void);
void test_collision(void);

int main(void) {
  static const char text[] = "Sprite Collisions";

  static const char palette_bg[16] = {
      0x00, 0x00, 0x10, 0x30 // gray, gray, lt gray, white
  };

  static const char palette_sp[16] = {
      0x0f, 0x0f, 0x0f, 0x28, // black, black, yellow
      0x0f, 0x0f, 0x0f, 0x12, // black, black, blue
  };

  ppu_off(); // screen off

  // load the palettes
  pal_bg(palette_bg);
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprites are set to 0 by default
  bank_spr(1);

  // load the text
  // vram_adr(NTADR_A(x,y));
  vram_adr(NTADR_A(7, 14)); // set a start position for the text

  // vram_write draws the array to the screen
  vram_write(text, sizeof(text));

  // turn on screen
  ppu_on_all();

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame
    // the sprites are pushed from a buffer to the OAM during nmi

    movement();
    test_collision();
    draw_sprites();
  }
}

void draw_sprites(void) {
  // clear all sprites from sprite buffer
  oam_clear();

  // draw 2 metasprites
  oam_meta_spr(BoxGuy1.x, BoxGuy1.y, YellowSpr);

  oam_meta_spr(BoxGuy2.x, BoxGuy2.y, BlueSpr);
}

void movement(void) {
  char pad1 = pad_poll(0); // read the first controller
  char pad2 = pad_poll(1); // read the second controller

  if (pad1 & PAD_LEFT) {
    BoxGuy1.x -= 1;
  } else if (pad1 & PAD_RIGHT) {
    BoxGuy1.x += 1;
  }
  if (pad1 & PAD_UP) {
    BoxGuy1.y -= 1;
  } else if (pad1 & PAD_DOWN) {
    BoxGuy1.y += 1;
  }

  if (pad2 & PAD_LEFT) {
    BoxGuy2.x -= 1;
  } else if (pad2 & PAD_RIGHT) {
    BoxGuy2.x += 1;
  }
  if (pad2 & PAD_UP) {
    BoxGuy2.y -= 1;
  } else if (pad2 & PAD_DOWN) {
    BoxGuy2.y += 1;
  }
}

void test_collision(void) {
  char collision = check_collision(&BoxGuy1, &BoxGuy2);

  // change the BG color, if sprites are touching
  if (collision) {
    pal_col(0, 0x30);
  } else {
    pal_col(0, 0x00);
  }
}
