/*	example code for llvm-mos, for NES
 *  Scroll Up with metatile system
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
#define MAX_UP 0x4000

// Horizontal mirroring.
asm(".globl __mirroring\n__mirroring = 0\n");

struct Hero {
  unsigned x; // low byte is sub-pixel
  unsigned y;
  int vel_x; // speed, signed, low byte is sub-pixel
  int vel_y;
};

struct Hero BoxGuy1 = {0x7800, 0xc400}; // starting position
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
void draw_screen_U(void);
void new_cmap(void);
char bg_collision_sub(char x, unsigned y);

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

  scroll_y = MAX_SCROLL; // start at MAX, downward to zero
  set_scroll_y(scroll_y);

  ppu_on_all(); // turn on screen

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame

    pad1 = pad_poll(0); // read the first controller

    movement();
    // set scroll
    set_scroll_x(scroll_x);
    set_scroll_y(scroll_y);
    draw_screen_U();
    draw_sprites();
  }
}

void load_room(void) {
  set_data_pointer(Rooms[MAX_ROOMS]);

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
  set_mt_pointer(metatiles1);

  char nt = (MAX_SCROLL >> 8) + 1;
  nt = (nt & 1) << 1;
  for (char y = 0;; y += 0x20) {
    for (char x = 0;; x += 0x20) {
      buffer_4_mt(get_ppu_addr(nt, x, y),
                  (y & 0xf0) + (x >> 4)); // ppu_address, index to the data
      flush_vram_update2();
      if (x == 0xe0)
        break;
    }
    if (y == 0xe0)
      break;
  }

  nt = nt ^ 2; // flip that 0000 0010 bit
  // a little bit in the other room
  set_data_pointer(Rooms[MAX_ROOMS - 1]);
  for (char x = 0;; x += 0x20) {
    char y = 0xe0;
    unsigned address = get_ppu_addr(nt, x, y);
    char index = (y & 0xf0) + (x >> 4);
    buffer_4_mt(address, index); // ppu_address, index to the data
    flush_vram_update2();
    if (x == 0xe0)
      break;
  }

  // copy the room to the collision map
  memcpy(c_map, Rooms[MAX_ROOMS], 240);
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
    if (BoxGuy1.x >= 0xf100) {
      BoxGuy1.vel_x = 0;
      BoxGuy1.x = 0xf100;
    } else if (BoxGuy1.x >
               0xed00) { // don't want to wrap around to the other side
      BoxGuy1.vel_x = 0x100;
    } else {
      BoxGuy1.vel_x = SPEED;
    }
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
    BoxGuy1.vel_y = -SPEED;
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

  // scroll
  unsigned new_y = BoxGuy1.y;
  if (BoxGuy1.y < MAX_UP) {
    char scroll_amt = (MAX_UP - BoxGuy1.y + 0x80) >>
            8; // the numbers work better with +80 (like 0.5)
    scroll_y = sub_scroll_y(scroll_amt, scroll_y);
    BoxGuy1.y += scroll_amt << 8;
  }

  if ((high_byte(scroll_y) >= 0x80) ||
      (scroll_y <= MIN_SCROLL)) { // 0x80 = negative
    scroll_y = MIN_SCROLL;        // stop scrolling up, end of level
    BoxGuy1.y = new_y;
    if (high_byte(BoxGuy1.y) < 2)
      BoxGuy1.y = 0x0200;
    if (high_byte(BoxGuy1.y) > 0xf0)
      BoxGuy1.y = 0x0200; // > 0xf0 wrapped to bottom
  }

  // do we need to load a new collision map? (scrolled into a new room)
  if ((scroll_y & 0xff) >= 0xec) {
    new_cmap();
  }
}

void bg_collision(char x, char y, char width, char height) {
  // note, !0 = collision
  // sprite collision with backgrounds
  // load the object's x,y,width,height to Generic, then call this

  collision_L = 0;
  collision_R = 0;
  collision_U = 0;
  collision_D = 0;

  char y_tmp = y;
  if (L_R_switch)
    y_tmp += 2; // fix bug, walking through walls

  if (y_tmp >= 0xf0)
    return;

  unsigned y_upper_left = add_scroll_y(y_tmp, scroll_y);
  char x_left = x;

  eject_L = x_left | 0xf0;
  eject_U = y_upper_left | 0xf0;

  if (bg_collision_sub(x_left, y_upper_left)) { // find a corner in the collision map
    ++collision_L;
    ++collision_U;
  }

  char x_right = x_left + width;

  eject_R = (x_right + 1) & 0x0f;

  if (bg_collision_sub(x_right,y_upper_left)) { // find a corner in the collision map
    ++collision_R;
    ++collision_U;
  }

  // again, lower

  // bottom right, x hasn't changed

  char y_bot_tmp = y + height; // y bottom
  if (L_R_switch)
    y_bot_tmp -= 2;                          // fix bug, walking through walls
  unsigned y_bot = add_scroll_y(y_bot_tmp, scroll_y); // upper left
  char y_bot_low = y_bot & 0xff;                  // low byte y

  eject_D = (y_bot_low + 1) & 0x0f;
  if (y_bot_low >= 0xf0)
    return;

  if (bg_collision_sub(x_right, y_bot)) { // find a corner in the collision map
    ++collision_R;
    ++collision_D;
  }

  // bottom left
  if (bg_collision_sub(x, y_bot)) { // find a corner in the collision map
    ++collision_L;
    ++collision_D;
  }
}

char bg_collision_sub(char x, unsigned y) {
  char upper_left = (x >> 4) + (y & 0xf0);
  if (y & (1 << 8)) {
    return c_map2[upper_left];
  } else {
    return c_map[upper_left];
  }
}

void draw_screen_U(void) {
  unsigned pseudo_scroll_y = sub_scroll_y(0x20, scroll_y);

  char room = pseudo_scroll_y >> 8;

  set_data_pointer(Rooms[room]);
  char nt = (room & 1) << 1; // 0 or 2
  char y = pseudo_scroll_y & 0xff;

  // important that the main loop clears the vram_buffer

  unsigned address = get_ppu_addr(nt, 0x40 * scroll_count, y);
  char index = (y & 0xf0) + 4 * scroll_count;
  buffer_4_mt(address, index); // ppu_address, index to the data

  address = get_ppu_addr(nt, 0x40 * scroll_count + 0x20, y);
  index = (y & 0xf0) + 4 * scroll_count + 2;
  buffer_4_mt(address, index); // ppu_address, index to the data

  ++scroll_count;
  scroll_count &= 3; // mask off top bits, keep it 0-3
}

// copy a new collision map to one of the 2 c_map arrays
void new_cmap(void) {
  // copy a new collision map to one of the 2 c_map arrays
  char room = scroll_y >> 8; // high byte = room, one to the right

  memcpy(room & 1 ? c_map2 : c_map, Rooms[room], 240);
}
