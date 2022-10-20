/*	example code for llvm-mos, for NES
 *  Scrolling Right with metatile system
 *	, basic platformer
 *	, adding music
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
#define ACCEL 0x20
#define GRAVITY 0x50
#define MAX_SPEED 0x240
#define JUMP_VEL (-0x600)
#define HERO_WIDTH 13
#define HERO_HEIGHT 11
#define MAX_RIGHT 0xb000
#define COL_DOWN 0x80
#define COL_ALL 0x40

struct Hero {
  unsigned x; // low byte is sub-pixel
  unsigned y;
  int vel_x; // speed, signed, low byte is sub-pixel
  int vel_y;
};

struct Hero BoxGuy1 = {0x4000, 0xc400}; // starting position
// the width and height should be 1 less than the dimensions (14x12)
// note, I'm using the top left as the 0,0 on x,y

static char c_map[240];
static char c_map2[240];

static char pad1;
static char pad1_new;
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

char song;
#define MAX_SONGS 2
enum { SONG_GAME, SONG_PAUSE };

void load_room(void);
void draw_sprites(void);
void movement(void);
void bg_collision(char x, char y, char width, char height);
void draw_screen_R(void);
void new_cmap(void);
char bg_collision_sub(unsigned x, char y);
void bg_check_low(char x, char y, char width, char height);
void change_song(void);

extern const char music_data[];

int main(void) {
  music_init(music_data);

  ppu_off(); // screen off

  // load the palettes
  // clang-format off
  static const char palette_bg[16] = {
    0x22, 0x16, 0x36, 0x0f,
    0,    8,    0x18, 0x39,
    0,    0,    0x10, 0x20,
    0,    0x0a, 0x1a, 0x2a,
  };
  // clang-format on
  pal_bg(palette_bg);
  // clang-format off
  static const char palette_sp[16] = {
    0x22, 0x01, 0x11, 0x10,
    0x22, 0x17, 0x28, 0x38,
    0x22, 0x05, 0x15, 0x35,
    0x22, 0x0f, 0x00, 0x30,
  };
  // clang-format on
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprites are set to 0 by default
  bank_spr(1);

  set_vram_buffer(); // do at least once

  load_room();
  music_play(song);

  ppu_on_all(); // turn on screen

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame

    set_music_speed(8);

    pad1 = pad_poll(0); // read the first controller
    pad1_new = get_pad_new(0);

    movement();
    // set scroll
    set_scroll_x(scroll_x);
    set_scroll_y(scroll_y);
    draw_screen_R();
    draw_sprites();

    change_song();
  }
}

void load_room(void) {
  set_data_pointer(Rooms[0]);

  // 5 bytes per metatile definition, tile TL, TR, BL, BR, palette 0-3
  // T means top, B means bottom, L left,R right
  // 51 maximum # of metatiles = 255 bytes
  // 5th byte, 	1000 0000 = floor collision
  // 				0100 0000 = all collision, solid
  //				0000 0011 = palette
  // clang-format off
  static const unsigned char metatiles1[]={
    0,  0,  0,  0,   0,
    2,  2,  2,  2,   3,
    20, 20, 20, 20,  0,
    5,  6,  21, 22,  1,
    6,  6,  22, 22,  1,
    6,  7,  22, 23,  1,
    21, 22, 21, 22,  1,
    22, 22, 22, 22,  1,
    22, 23, 22, 23,  1,
    8,  9,  24, 25,  1,
    9,  9,  25, 25,  1,
    9,  10, 25, 26,  1,
  };
  // clang-format off
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
      BoxGuy1.vel_x -= ACCEL;
      if (BoxGuy1.vel_x < -MAX_SPEED)
        BoxGuy1.vel_x = -MAX_SPEED;
    }
  } else if (pad1 & PAD_RIGHT) {

    direction = RIGHT;

    BoxGuy1.vel_x += ACCEL;
    if (BoxGuy1.vel_x > MAX_SPEED)
      BoxGuy1.vel_x = MAX_SPEED;
  } else { // nothing pressed
    if (BoxGuy1.vel_x >= 0x100)
      BoxGuy1.vel_x -= ACCEL;
    else if (BoxGuy1.vel_x < -0x100)
      BoxGuy1.vel_x += ACCEL;
    else
      BoxGuy1.vel_x = 0;
  }

  BoxGuy1.x += BoxGuy1.vel_x;

  if (BoxGuy1.x > 0xf800) { // make sure no wrap around to the other side
    BoxGuy1.x = 0x100;
    BoxGuy1.vel_x = 0;
  }

  L_R_switch = 1; // shinks the y values in bg_coll, less problems with head /
                  // feet collisions

  bg_collision(high_byte(BoxGuy1.x), high_byte(BoxGuy1.y), HERO_WIDTH,
               HERO_HEIGHT);
  if (collision_R &&
      collision_L) { // if both true, probably half stuck in a wall
    BoxGuy1.x = old_x;
    BoxGuy1.vel_x = 0;
  } else if (collision_L) {
    BoxGuy1.vel_x = 0;
    high_byte(BoxGuy1.x) = high_byte(BoxGuy1.x) - eject_L;

  } else if (collision_R) {
    BoxGuy1.vel_x = 0;
    high_byte(BoxGuy1.x) = high_byte(BoxGuy1.x) - eject_R;
  }

  // handle y

  // gravity

  // BoxGuy1.vel_y is signed
  if (BoxGuy1.vel_y < 0x300) {
    BoxGuy1.vel_y += GRAVITY;
  } else {
    BoxGuy1.vel_y = 0x300; // consistent
  }
  BoxGuy1.y += BoxGuy1.vel_y;

  L_R_switch = 0;
  bg_collision(high_byte(BoxGuy1.x), high_byte(BoxGuy1.y), HERO_WIDTH,
               HERO_HEIGHT);

  if (collision_U) {
    high_byte(BoxGuy1.y) -= eject_U;
    BoxGuy1.vel_y = 0;
  } else if (collision_D) {
    high_byte(BoxGuy1.y) -= eject_D;
    BoxGuy1.y &= 0xff00;
    if (BoxGuy1.vel_y > 0) {
      BoxGuy1.vel_y = 0;
    }
  }

  // check collision down a little lower than hero
  bg_check_low(high_byte(BoxGuy1.x), high_byte(BoxGuy1.y), HERO_WIDTH,
               HERO_HEIGHT);
  if (collision_D) {
    if (pad1_new & PAD_A) {
      BoxGuy1.vel_y = JUMP_VEL; // JUMP
    }
  }

  // do we need to load a new collision map? (scrolled into a new room)
  if ((scroll_x & 0xff) < 4) {
    new_cmap();
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
  // note, uses bits in the metatile data to determine collision
  // sprite collision with backgrounds

  collision_L = 0;
  collision_R = 0;
  collision_U = 0;
  collision_D = 0;

  if (y >= 0xf0)
    return;

  unsigned x_upper_left = x + scroll_x; // upper left (temp6 = save for reuse)

  eject_L = (x_upper_left & 0xff) | 0xf0;

  char y_top = y;

  eject_U = y_top | 0xf0;

  if (L_R_switch)
    y_top += 2; // fix bug, walking through walls

  if (bg_collision_sub(x_upper_left,
                       y_top) & COL_ALL) { // find a corner in the collision map
    ++collision_L;
    ++collision_U;
  }

  unsigned x_upper_right = x_upper_left + width;

  eject_R = (x_upper_right + 1) & 0x0f;

  // find a corner in the collision map
  if (bg_collision_sub(x_upper_right, y_top) & COL_ALL) {
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

  char collision = bg_collision_sub(x_upper_right, y_bot);

  if (collision & COL_ALL) { // find a corner in the collision map
    ++collision_R;
  }
  if (collision & (COL_DOWN | COL_ALL)) { // find a corner in the collision map
    ++collision_D;
  }

  // bottom left
  collision = bg_collision_sub(x_upper_left, y_bot);

  if (collision & COL_ALL) { // find a corner in the collision map
    ++collision_L;
  }
  if (collision & (COL_DOWN | COL_ALL)) { // find a corner in the collision map
    ++collision_D;
  }

  if ((y_bot & 0x0f) > 3)
    collision_D = 0; // for platforms, only collide with the top 3 pixels
}

char bg_collision_sub(unsigned x, char y) {
  char upper_left = ((x & 0xff) >> 4) + (y & 0xf0);
  char typ = (x & 1 << 8 ? c_map2 : c_map)[upper_left];
  static const char is_solid[]={
    0,
    COL_DOWN,
    COL_ALL+COL_DOWN,
    COL_DOWN,
    COL_DOWN,
    COL_DOWN,
    0,
    0,
    0,
    0,
    0,
    0
  };
  return is_solid[typ];
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

void new_cmap(void) {
  // copy a new collision map to one of the 2 c_map arrays
  char room = ((scroll_x >> 8) + 1); // high byte = room, one to the right
  char *map = room & 1 ? c_map2 : c_map;
  memcpy(map, Rooms[room], 240);
}

void bg_check_low(char x, char y, char width, char height) {
  // floor collisions
  collision_D = 0;

  unsigned x_left = x + scroll_x;
  char y_bot = y + height + 1;

  if (y_bot >= 0xf0)
    return;

  // find a corner in the collision map
  if (bg_collision_sub(x_left, y_bot) & (COL_DOWN | COL_ALL)) {
    ++collision_D;
  }

  unsigned x_right = x_left + width;
  // find a corner in the collision map
  if (bg_collision_sub(x_right, y_bot) & (COL_DOWN | COL_ALL)) {
    ++collision_D;
  }

  if ((y_bot & 0x0f) > 3)
    collision_D = 0; // for platforms, only collide with the top 3 pixels
}

void change_song(void) {
  if (pad1_new & PAD_START) {
    if (++song >= MAX_SONGS)
      song = 0;
    music_play(song);
  }
}
