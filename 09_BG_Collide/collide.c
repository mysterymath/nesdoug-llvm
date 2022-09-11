/*	example code for llvm-mos, for NES
 *  move some sprites with the controllers
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>
#include <string.h>

#include "CSV/c.h"
#include "Sprites.h" // holds our metasprite data

// collision map
char c_map[240];

struct BoxGuy {
  char X;
  char Y;
  char width;
  char height;
};

struct BoxGuy BoxGuy1 = {64, 80, 15, 15};

struct Collision {
  char L;
  char R;
  char U;
  char D;
};

void draw_bg(void);
void draw_sprites(void);
void movement(char pad1);
struct Collision bg_collision(char *object);
void check_start(char pad1_new);

char which_bg;

int main(void) {
  static const char palette_bg[16] = {
      0x0f, 0x00, 0x10, 0x30, // black, gray, lt gray, white
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

  set_scroll_y(0xff); // shift the bg down 1 pixel

  draw_bg();

  // turn on screen
  // ppu_on_all(); //already done in draw_bg()

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame
    // the sprites are pushed from a buffer to the OAM during nmi

    char pad1 = pad_poll(0);        // read the first controller
    char pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first

    movement(pad1);
    draw_sprites();
    check_start(pad1_new);
  }
}

void draw_bg(void) {
  static const unsigned char *const All_Collision_Maps[] = {c1, c2, c3, c4};

  ppu_off(); // screen off

  const unsigned char *p_maps = All_Collision_Maps[which_bg];
  // copy the collision map to c_map
  memcpy(c_map, p_maps, 240);

  // this sets a start position on the BG, top left of screen
  vram_adr(NAMETABLE_A);

  // draw the tiles
  for (char y = 0; y < 15; ++y) {
    for (char x = 0; x < 16; ++x) {
      if (c_map[(y << 4) + x]) {
        vram_put(0x10); // wall
        vram_put(0x10);
      } else {
        vram_put(0); // blank
        vram_put(0);
      }
    }

    // do twice
    for (char x = 0; x < 16; ++x) {
      if (c_map[(y << 4) + x]) {
        vram_put(0x10); // wall
        vram_put(0x10);
      } else {
        vram_put(0); // blank
        vram_put(0);
      }
    }
  }

  ppu_on_all(); // turn on screen
}

void draw_sprites(void) {
  // clear all sprites from sprite buffer
  oam_clear();

  // draw 1 metasprite
  oam_meta_spr(BoxGuy1.X, BoxGuy1.Y, YellowSpr);
}

void movement(char pad1) {
  if (pad1 & PAD_LEFT) {
    BoxGuy1.X -= 1;
  } else if (pad1 & PAD_RIGHT) {
    BoxGuy1.X += 1;
  }

  struct Collision collision = bg_collision((char *)&BoxGuy1);
  if (collision.R)
    BoxGuy1.X -= 1;
  if (collision.L)
    BoxGuy1.X += 1;

  if (pad1 & PAD_UP) {
    BoxGuy1.Y -= 1;
  } else if (pad1 & PAD_DOWN) {
    BoxGuy1.Y += 1;
  }

  collision = bg_collision((char *)&BoxGuy1);
  if (collision.D)
    BoxGuy1.Y -= 1;
  if (collision.U)
    BoxGuy1.Y += 1;
}

struct Collision bg_collision(char *object) {
  // sprite collision with backgrounds
  // object expected to have first 4 bytes as x,y,width,height
  // casting to char* so this function could work for any sized structs
  struct Collision collision = {0};

  char left = object[0];         // left side
  char right = left + object[2]; // right side
  char top = object[1];          // top side
  char bot = top + object[3];    // bottom side

  if (top >= 0xf0)
    return collision;
  // y out of range

  // upper left
  if (c_map[(left >> 4) + (top & 0xf0)]) { // find a corner in the collision map
    ++collision.L;
    ++collision.U;
  }

  // upper right
  if (c_map[(right >> 4) + (top & 0xf0)]) {
    ++collision.R;
    ++collision.U;
  }

  if (bot >= 0xf0)
    return collision;
  // y out of range

  // bottom left
  if (c_map[(left >> 4) + (bot & 0xf0)]) {
    ++collision.L;
    ++collision.D;
  }

  // bottom right
  if (c_map[(right >> 4) + (bot & 0xf0)]) {
    ++collision.R;
    ++collision.D;
  }

  return collision;
}

void check_start(char pad1_new) {
  // if START is pressed, load another background
  if (pad1_new & PAD_START) {
    ++which_bg;
    if (which_bg >= 4)
      which_bg = 0;
    draw_bg();
  }
}
