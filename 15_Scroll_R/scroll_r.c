/*	example code for cc65, for NES
 *  Scroll Right with metatile system
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>
#include <string.h>

#include "BG/Rooms.h"
#include "Sprites.h" // holds our metasprite data

#define LEFT 0
#define RIGHT 1
#define SPEED 0x180
#define HERO_WIDTH 13
#define HERO_HEIGHT 13
#define MAX_RIGHT 0xb000

struct Hero {
  unsigned x; // low byte is sub-pixel
  unsigned y;
  int vel_x; // speed, signed, low byte is sub-pixel
  int vel_y;
};

struct Hero BoxGuy1 = {0x4000, 0xc400}; // starting position
// the width and height should be 1 less than the dimensions (14x14)
// note, I'm using the top left as the 0,0 on x,y

static char c_map[240];
static char c_map2[240];

static char pad1;
static char collision_L;
static char collision_R;
static char collision_U;
static char collision_D;
static char eject_L;   // from the left
static char eject_R;   // remember these from the collision sub routine
static char eject_D;   // from below
static char eject_U;   // from up
static char direction; // facing left or right?

static unsigned scroll_x;
static unsigned scroll_y;
static char scroll_count;
static char L_R_switch;

void load_room(void);
void draw_sprites(void);
void movement(void);
void bg_collision(char x, char y, char width, char height);
void draw_screen_R(void);
void new_cmap(void);
char bg_collision_sub(unsigned x, char y);

int main(void) {

  ppu_off(); // screen off

  // load the palettes
  static const char palette_bg[] = {
      0x0f, 0x00, 0x10, 0x30, // black, gray, lt gray, white
      0x0f, 0x07, 0x17, 0x27, // oranges
      0x0f, 0x02, 0x12, 0x22, // blues
      0x0f, 0x09, 0x19, 0x29, // greens
  };
  pal_bg(palette_bg);

  static const char palette_sp[16] = {
      0x0f, 0x07, 0x28, 0x38 // dk brown, yellow, white yellow
  };
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprites are set to 0 by default
  bank_spr(1);

  set_vram_buffer(); // do at least once, sets a pointer to a buffer

  load_room();

  scroll_y = 0xff;
  set_scroll_y(scroll_y); // shift the bg down 1 pixel

  ppu_on_all(); // turn on screen

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame

    pad1 = pad_poll(0); // read the first controller

    movement();
    // set scroll
    set_scroll_x(scroll_x);
    set_scroll_y(scroll_y);
    draw_screen_R();
    draw_sprites();
  }
}

void load_room(void) {
  set_data_pointer(Rooms[0]);

  // 5 bytes per metatile definition, tile TL, TR, BL, BR, palette 0-3
  // T means top, B means bottom, L left,R right
  // 51 maximum # of metatiles = 255 bytes
  // clang-format off
  static const char metatiles1[] = {
    2, 2, 2, 2, 3,
    4, 4, 4, 4, 1,
    9, 9, 9, 9, 2,
    5, 6, 8, 7, 1,
    5, 6, 8, 7, 0
  };
  // clang-format on
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

  // a little bit in the next room
  set_data_pointer(Rooms[1]);
  for (char y = 0;; y += 0x20) {
    char x = 0;
    buffer_4_mt(get_ppu_addr(1, x, y),
                (y & 0xf0)); // ppu_address, index to the data
    flush_vram_update2();
    if (y == 0xe0)
      break;
  }

  // copy the room to the collision map
  // the second one should auto-load with the scrolling code
  memcpy(c_map, Rooms[0], 240);
}

void draw_sprites(void) {
  // clear all sprites from sprite buffer
  oam_clear();

  // draw 1 metasprite
  if (direction == LEFT) {
    oam_meta_spr(high_byte(BoxGuy1.x), high_byte(BoxGuy1.y), RoundSprL);
  } else {
    oam_meta_spr(high_byte(BoxGuy1.x), high_byte(BoxGuy1.y), RoundSprR);
  }
}

void movement(void) {

  // handle x

  char old_x = BoxGuy1.x;

  if (pad1 & PAD_LEFT) {
    direction = LEFT;
    if (BoxGuy1.x <= 0x100) {
      BoxGuy1.vel_x = 0;
      BoxGuy1.x = 0x100;
    } else if (BoxGuy1.x <
               0x400) { // don't want to wrap around to the other side
      BoxGuy1.vel_x = -0x100;
    } else {
      BoxGuy1.vel_x = -SPEED;
    }
  } else if (pad1 & PAD_RIGHT) {

    direction = RIGHT;

    BoxGuy1.vel_x = SPEED;
  } else { // nothing pressed
    BoxGuy1.vel_x = 0;
  }

  BoxGuy1.x += BoxGuy1.vel_x;

  if ((BoxGuy1.x < 0x100) ||
      (BoxGuy1.x > 0xf800)) { // make sure no wrap around to the other side
    BoxGuy1.x = 0x100;
  }

  L_R_switch = 1; // shinks the y values in bg_coll, less problems with head /
                  // feet collisions

  bg_collision(high_byte(BoxGuy1.x), high_byte(BoxGuy1.y), HERO_WIDTH,
               HERO_HEIGHT);
  if (collision_R &&
      collision_L) { // if both true, probably half stuck in a wall
    BoxGuy1.x = old_x;
  } else if (collision_L) {
    high_byte(BoxGuy1.x) -= eject_L;

  } else if (collision_R) {
    high_byte(BoxGuy1.x) -= eject_R;
  }

  // handle y
  char old_y = BoxGuy1.y;

  if (pad1 & PAD_UP) {
    if (BoxGuy1.y <= 0x100) {
      BoxGuy1.vel_y = 0;
      BoxGuy1.y = 0x100;
    } else if (BoxGuy1.y <
               0x400) { // don't want to wrap around to the other side
      BoxGuy1.vel_y = -0x100;
    } else {
      BoxGuy1.vel_y = -SPEED;
    }
  } else if (pad1 & PAD_DOWN) {
    if (BoxGuy1.y >= 0xe000) {
      BoxGuy1.vel_y = 0;
      BoxGuy1.y = 0xe000;
    } else if (BoxGuy1.y >
               0xdc00) { // don't want to wrap around to the other side
      BoxGuy1.vel_y = 0x100;
    } else {
      BoxGuy1.vel_y = SPEED;
    }
  } else { // nothing pressed
    BoxGuy1.vel_y = 0;
  }

  BoxGuy1.y += BoxGuy1.vel_y;

  if ((BoxGuy1.y < 0x100) ||
      (BoxGuy1.y > 0xf000)) { // make sure no wrap around to the other side
    BoxGuy1.y = 0x100;
  }

  L_R_switch = 0; // shinks the y values in bg_coll, less problems with head /
                  // feet collisions

  bg_collision(high_byte(BoxGuy1.x), high_byte(BoxGuy1.y), HERO_WIDTH,
               HERO_HEIGHT);
  if (collision_U &&
      collision_D) { // if both true, probably half stuck in a wall
    BoxGuy1.y = old_y;
  } else if (collision_U) {
    high_byte(BoxGuy1.y) -= eject_U;

  } else if (collision_D) {
    high_byte(BoxGuy1.y) -= eject_D;
  }

  // do we need to load a new collision map? (scrolled into a new room)
  if ((scroll_x & 0xff) < 4) {
    new_cmap(); //
  }

  // scroll
  unsigned new_x = BoxGuy1.x;
  if (BoxGuy1.x > MAX_RIGHT) {
    char scroll_amt = (BoxGuy1.x - MAX_RIGHT) >> 8;
    scroll_x += scroll_amt;
    high_byte(BoxGuy1.x) -= scroll_amt;
  }

  if (scroll_x >= MAX_SCROLL) {
    scroll_x = MAX_SCROLL; // stop scrolling right, end of level
    BoxGuy1.x = new_x;     // but allow the x position to go all the way right
    if (high_byte(BoxGuy1.x) >= 0xf1) {
      BoxGuy1.x = 0xf100;
    }
  }
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
  char upper_left = ((x & 0xff) >> 4) + (y & 0xf0);
  if (x & (1 << 8)) {
    return c_map2[upper_left];
  } else {
    return c_map[upper_left];
  }
}

void draw_screen_R(void) {
  // scrolling to the right, draw metatiles as we go
  unsigned pseudo_scroll_x = scroll_x + 0x120;

  char room = pseudo_scroll_x >> 8;

  set_data_pointer(Rooms[room]);
  char nt = room & 1;
  char x = pseudo_scroll_x & 0xff;

  // important that the main loop clears the vram_buffer

  // Note: This becomes a shift, since 0x40 is a power of 2.
  char offset = scroll_count * 0x40;
  buffer_4_mt(get_ppu_addr(nt, x, offset),
              offset + (x >> 4)); // ppu_address, index to the data
  buffer_4_mt(get_ppu_addr(nt, x, offset + 0x20),
              offset + 0x20 + (x >> 4)); // ppu_address, index to the data

  ++scroll_count;
  scroll_count &= 3; // mask off top bits, keep it 0-3
}

// copy a new collision map to one of the 2 c_map arrays
void new_cmap(void) {
  // copy a new collision map to one of the 2 c_map arrays
  char room = ((scroll_x >> 8) + 1); // high byte = room, one to the right
  if (!(room & 1)) {                 // even or odd?
    memcpy(c_map, Rooms[room], 240);
  } else {
    memcpy(c_map2, Rooms[room], 240);
  }
}
