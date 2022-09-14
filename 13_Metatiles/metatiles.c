/*	example code for llvm-mos, for NES
 *  draw a BG with metatile system
 *	, also sprite collisions with BG
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>
#include <stddef.h>
#include <string.h>

#include "BG/Room1.h"
#include "Sprites.h" // holds our metasprite data

#define LEFT 0
#define RIGHT 1
#define SPEED 0x180
#define HERO_WIDTH 13
#define HERO_HEIGHT 13

struct BoxGuy {
  char x;
  char y;
} BoxGuy1 = {0x40, 0x30};
// the width and height should be 1 less than the dimensions (16x16)
// ...I shrunk it a bit 14x14 hitbox
// note, I'm using the top left as the 0,0 on x,y

static char c_map[240];
static char c_map2[240]; // not used in this example

static char pad1;
static char collision_L;
static char collision_R;
static char collision_U;
static char collision_D;
static char coordinates;
static char eject_L;   // from the left
static char eject_R;   // remember these from the collision sub routine
static char eject_D;   // from below
static char eject_U;   // from up
static char direction; // facing left or right?

static char scroll_x;
static char scroll_y;
static int hero_velocity_x; // signed, low byte is sub-pixel
static int hero_velocity_y;
static unsigned hero_x;
static unsigned hero_y;
static char L_R_switch;

void load_room(void);
void draw_sprites(void);
void movement(void);
void bg_collision(char x, char y, char width, char height);
char bg_collision_sub(unsigned x, char y);

int main(void) {

  ppu_off(); // screen off

  // load the palettes
  static const unsigned char palette_bg[] = {
      0x0f, 0x00, 0x10, 0x30, // black, gray, lt gray, white
      0x0f, 0x07, 0x17, 0x27, // oranges
      0x0f, 0x02, 0x12, 0x22, // blues
      0x0f, 0x09, 0x19, 0x29, // greens
  };
  pal_bg(palette_bg);

  static const unsigned char palette_sp[16] = {
      0x0f, 0x07, 0x28, 0x38, // dk brown, yellow, white yellow
  };
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprites are set to 0 by default
  bank_spr(1);

  set_vram_buffer(); // do at least once, sets a pointer to a buffer

  load_room();

  set_scroll_y(0xff); // shift the bg down 1 pixel

  ppu_on_all(); // turn on screen

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame

    pad1 = pad_poll(0); // read the first controller

    movement();
    draw_sprites();
  }
}

void load_room(void) {
  set_data_pointer(Room1);

  // 5 bytes per metatile definition, tile TL, TR, BL, BR, palette 0-3
  // T means top, B means bottom, L left,R right
  // 51 maximum # of metatiles = 255 bytes
  static const char metatiles1[] = {2, 2, 2, 2, 3, 4, 4, 4, 4, 1, 9, 9, 9,
                                    9, 2, 5, 6, 8, 7, 1, 5, 6, 8, 7, 0};
  set_mt_pointer(metatiles1);
  for (char y = 0;; y += 0x20) {
    for (char x = 0;; x += 0x20) {
      buffer_4_mt(get_ppu_addr(0, x, y),
                  (y & 0xf0) + (x >> 4)); // ppu_address, index to the data
      flush_vram_update2();
      if (x == 0xe0)
        break;
    }
    if (y == 0xe0)
      break;
  }

  set_vram_update(NULL); // just turn ppu updates OFF for this example

  // copy the room to the collision map
  memcpy(c_map, Room1, 240);

  hero_y = BoxGuy1.y << 8;
  hero_x = BoxGuy1.x << 8;
}

void draw_sprites(void) {
  // clear all sprites from sprite buffer
  oam_clear();

  // draw 1 metasprite
  if (direction == LEFT) {
    oam_meta_spr(BoxGuy1.x, BoxGuy1.y, RoundSprL);
  } else {
    oam_meta_spr(BoxGuy1.x, BoxGuy1.y, RoundSprR);
  }
}

void movement(void) {
  // handle x
  char old_x = BoxGuy1.x;

  if (pad1 & PAD_LEFT) {
    direction = LEFT;
    if (BoxGuy1.x <= 1) {
      hero_velocity_x = 0;
      hero_x = 0x100;
    } else if (BoxGuy1.x < 4) { // don't want to wrap around to the other side
      hero_velocity_x = -0x100;
    } else {
      hero_velocity_x = -SPEED;
    }
  } else if (pad1 & PAD_RIGHT) {
    direction = RIGHT;
    if (BoxGuy1.x >= 0xf1) {
      hero_velocity_x = 0;
      hero_x = 0xf100;
    } else if (BoxGuy1.x >
               0xec) { // don't want to wrap around to the other side
      hero_velocity_x = 0x100;
    } else {
      hero_velocity_x = SPEED;
    }
  } else { // nothing pressed
    hero_velocity_x = 0;
  }

  hero_x += hero_velocity_x;
  BoxGuy1.x = hero_x >> 8; // the collision routine needs an 8 bit value

  L_R_switch = 1; // shinks the y values in bg_coll, less problems with head /
                  // feet collisions

  bg_collision(BoxGuy1.x, BoxGuy1.y, HERO_WIDTH, HERO_HEIGHT);
  if (collision_R &&
      collision_L) { // if both true, probably half stuck in a wall
    BoxGuy1.x = old_x;
  } else if (collision_L) {
    BoxGuy1.x -= eject_L;
  } else if (collision_R) {
    BoxGuy1.x -= eject_R;
  }
  high_byte(hero_x) = BoxGuy1.x;

  if (pad1 & PAD_UP) {
    if (BoxGuy1.y <= 1) {
      hero_velocity_y = 0;
      hero_y = 0x100;
    } else if (BoxGuy1.y < 4) { // don't want to wrap around to the other side
      hero_velocity_y = -0x100;
    } else {
      hero_velocity_y = -SPEED;
    }
  } else if (pad1 & PAD_DOWN) {
    if (BoxGuy1.y >= 0xe0) {
      hero_velocity_y = 0;
      hero_y = 0xe000;
    } else if (BoxGuy1.y >
               0xdc) { // don't want to wrap around to the other side
      hero_velocity_y = 0x100;
    } else {
      hero_velocity_y = SPEED;
    }
  } else { // nothing pressed
    hero_velocity_y = 0;
  }

  hero_y += hero_velocity_y;
  BoxGuy1.y = hero_y >> 8;
  // the collision routine needs an 8 bit value

  L_R_switch = 0;

  bg_collision(BoxGuy1.x, BoxGuy1.y, HERO_WIDTH, HERO_HEIGHT);

  if (collision_U) {
    BoxGuy1.y -= eject_U;
  } else if (collision_D) {
    BoxGuy1.y -= eject_D;
  }
  high_byte(hero_y) = BoxGuy1.y;
}

void bg_collision(char x, char y, char width, char height) {
  // note, !0 = collision
  // sprite collision with backgrounds

  collision_L = 0;
  collision_R = 0;
  collision_U = 0;
  collision_D = 0;

  // this was borrowed from a multi-screen engine, so it handles
  // high bytes on position, even though they should always be zero here

  if (y >= 0xf0)
    return;

  unsigned x_upper_left = x + scroll_x; // upper left (temp6 = save for reuse)

  eject_L = (x_upper_left & 0xff) | 0xf0;

  char y_top = y;

  eject_U = y_top | 0xf0;

  if (L_R_switch)
    y_top += 2; // fix bug, walking through walls

  if (bg_collision_sub(x_upper_left,
                       y_top)) { // find a corner in the collision map
    ++collision_L;
    ++collision_U;
  }

  unsigned x_upper_right = x_upper_left + width;

  eject_R = (x_upper_right + 1) & 0x0f;

  // y_top is unchanged
  if (bg_collision_sub(x_upper_right,
                       y_top)) { // find a corner in the collision map
    ++collision_R;
    ++collision_U;
  }

  // again, lower

  // bottom right, x hasn't changed

  char y_bot = y + height; // y bottom
  if (L_R_switch)
    y_bot -= 2; // fix bug, walking through walls
  eject_D = (y_bot + 1) & 0x0f;
  if (y_bot >= 0xf0)
    return;

  if (bg_collision_sub(x_upper_right,
                       y_bot)) { // find a corner in the collision map
    ++collision_R;
    ++collision_D;
  }

  // bottom left
  // find a corner in the collision ap
  if (bg_collision_sub(x_upper_left, y_bot)) {
    ++collision_L;
    ++collision_D;
  }
}

char bg_collision_sub(unsigned x, char y) {
  // this was borrowed from a multi-screen engine
  // c_map2 should never be used here
  char upper_left = ((x & 0xff) >> 4) + (y & 0xf0);
  if (x & (1 << 8)) {
    return c_map2[upper_left];
  } else {
    return c_map[upper_left];
  }
}
