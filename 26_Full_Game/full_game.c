/*	example code for llvm-mos, for NES
 *  Scrolling Right with metatile system
 *	, basic platformer
 *	, with coins and enemies
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>
#include <stdbool.h>
#include <string.h>

#include "Sprites.h" // holds our metasprite data
#include "level_data.h"
#include "palettes.h"

#include "BG/Levels.h"
#include "NES_ST/title.h"

#define LEFT 0
#define RIGHT 1
#define ACCEL 0x30
#define GRAVITY 0x50
#define MAX_SPEED 0x240
#define JUMP_VEL (-0x600)
#define HERO_WIDTH 13
#define HERO_HEIGHT 11
#define MAX_RIGHT 0x9000
#define COL_DOWN 0x80
#define COL_ALL 0x40

struct Hero {
  unsigned x; // low byte is sub-pixel
  unsigned y;
  int vel_x; // speed, signed, low byte is sub-pixel
  int vel_y;
};

struct Hero BoxGuy1;

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
enum { SFX_JUMP, SFX_DING, SFX_NOISE };

char game_mode;
enum {
  MODE_TITLE,
  MODE_GAME,
  MODE_PAUSE,
  MODE_SWITCH,
  MODE_END,
  MODE_GAME_OVER
};

char coins;
char lives;

char level;
char level_up;
char death;
char map_loaded;   // only load it once
char enemy_frames; // in case of skipped frames

#define MAX_COINS 16
char coin_x[MAX_COINS];
char coin_y[MAX_COINS];
char coin_active[MAX_COINS];
char coin_room[MAX_COINS];
char coin_actual_x[MAX_COINS];
char coin_type[MAX_COINS];

#define COIN_WIDTH 7
#define COIN_HEIGHT 11

#define BIG_COIN 13

#define MAX_ENEMY 16
char enemy_x[MAX_ENEMY];
char enemy_y[MAX_ENEMY];
char enemy_active[MAX_ENEMY];
char enemy_room[MAX_ENEMY];
char enemy_actual_x[MAX_ENEMY];
char enemy_type[MAX_ENEMY];
const unsigned char *enemy_anim[MAX_ENEMY];

#define ENEMY_WIDTH 13
#define ENEMY_HEIGHT 13

void load_title(void);
void load_room(void);
void draw_sprites(void);
void movement(void);
void bg_collision(char x, char y, char width, char height);
void draw_screen_R(void);
void new_cmap(void);
char bg_collision_sub(unsigned x, char y);
void bg_check_low(char x, char y, char width, char height);
void enemy_moves(char index);
void sprite_collisions(void);
void check_spr_objects(void);
void sprite_obj_init(void);
void set_sprite_zero(void);
void update_hud(void);
void bg_collision_fast(char x, char y, char width);

extern const char music_data[];
extern const char sounds_data[];

int main(void) {
  music_init(music_data);
  sounds_init(sounds_data);

  ppu_off(); // screen off

  // use the second set of tiles for sprites
  // both bg and sprites are set to 0 by default
  bank_spr(1);

  set_vram_buffer(); // do at least once

  load_title();

  ppu_on_all(); // turn on screen

  scroll_x = 0;
  set_scroll_x(scroll_x);
#ifdef START_LEVEL
  level = START_LEVEL; // debugging, start on level...
#endif

  char bright = 0;
  char bright_count = 0;
  while (1) {
    while (game_mode == MODE_TITLE) {
      ppu_wait_nmi();
      set_music_speed(8);

      static const char title_color_rotate[] = {0x32, 0x22, 0x30, 0x37};
      pal_col(3, title_color_rotate[(get_frame_count() >> 3) & 3]);

      pad1 = pad_poll(0); // read the first controller
      pad1_new = get_pad_new(0);
      if (pad1_new & PAD_START) {
        pal_fade_to(4, 0); // fade to black
        ppu_off();
        load_room();
        game_mode = MODE_GAME;
        pal_bg(palette_bg);
        song = SONG_GAME;
        music_play(song);
#ifdef LIVES
        lives = 0; // removed this feature, but we could add it later
#endif
        scroll_x = 0;
        set_scroll_x(scroll_x);
        ppu_on_all();
        pal_bright(4); // back to normal brighness
      }
    }

    while (game_mode == MODE_GAME) {
      ppu_wait_nmi();

      set_music_speed(8);

      pad1 = pad_poll(0); // read the first controller
      pad1_new = get_pad_new(0);

      // there is a visual delay of 1 frame, so properly you should
      // 1. move user 2.check collisions 3.allow enemy moves 4.draw sprites
      movement();
      check_spr_objects(); // see which objects are on screen
      sprite_collisions();

      // set scroll
      set_scroll_x(scroll_x);
      draw_screen_R();
      draw_sprites();

      if (pad1_new & PAD_START) {
        game_mode = MODE_PAUSE;
        song = SONG_PAUSE;
        music_play(song);
        color_emphasis(COL_EMP_DARK);
        break; // out of the game loop
      }

#ifdef GRAY_LINE
      gray_line(); // debugging
#endif

      if (level_up) {
        game_mode = MODE_SWITCH;
        level_up = 0;
        bright = 4;
        bright_count = 0;
        ++level;
      } else if (death) {
        death = 0;
        bright = 0;
        scroll_x = 0;
        set_scroll_x(scroll_x);
        ppu_off();
        delay(5);

#ifdef LIVES
        --lives;            // removed feature
        if (lives > 0x80) { // negative, out of lives
#endif
          oam_clear();
          game_mode = MODE_GAME_OVER;
          vram_adr(NAMETABLE_A);
          vram_fill(0, 1024); // blank the screen
          ppu_on_all();
#ifdef LIVES
        } else {
          game_mode = MODE_SWITCH;
        }
#endif
      }
    }

    // switch rooms, due to level++
    // also, death, restart level (removed feature)
    while (game_mode == MODE_SWITCH) {
      ppu_wait_nmi();
      if (++bright_count >= 0x10) { // fade out
        bright_count = 0;
        if (--bright != 0xff)
          pal_bright(bright); // fade out
      }
      set_scroll_x(scroll_x);

      if (bright == 0xff) { // now switch rooms
        ppu_off();
        oam_clear();
        scroll_x = 0;
        set_scroll_x(scroll_x);
        if (level < 3) {
          load_room();
          game_mode = MODE_GAME;
          ppu_on_all();
          pal_bright(4); // back to normal brighness
        } else {         // set end of game. Did we win?
          game_mode = MODE_END;
          vram_adr(NAMETABLE_A);
          vram_fill(0, 1024);
          ppu_on_all();
          pal_bright(4); // back to normal brighness
        }
      }
    }

    while (game_mode == MODE_PAUSE) {
      ppu_wait_nmi();

      pad1 = pad_poll(0); // read the first controller
      pad1_new = get_pad_new(0);

      draw_sprites();

      if (pad1_new & PAD_START) {
        game_mode = MODE_GAME;
        song = SONG_GAME;
        music_play(song);
        color_emphasis(COL_EMP_NORMAL);
      }
    }

    while (game_mode == MODE_END) {
      ppu_wait_nmi();
      oam_clear();

      static const char END_TEXT[] = "The end of the game.";
      multi_vram_buffer_horz(END_TEXT, sizeof(END_TEXT), NTADR_A(6, 13));
      static const char END_TEXT2[] = "I guess you won?";
      multi_vram_buffer_horz(END_TEXT2, sizeof(END_TEXT2), NTADR_A(8, 15));
      static const char END_TEXT3[] = "Coins: ";
      multi_vram_buffer_horz(END_TEXT3, sizeof(END_TEXT3), NTADR_A(11, 17));
      one_vram_buffer((coins / 10) + 0x30, NTADR_A(18, 17));
      one_vram_buffer((coins % 10) + 0x30, NTADR_A(19, 17));

      set_scroll_x(0);

      music_stop();
    }

    while (game_mode == MODE_GAME_OVER) {
      ppu_wait_nmi();
      oam_clear();

      static const char DEAD_TEXT[] = "You died.";
      multi_vram_buffer_horz(DEAD_TEXT, sizeof(DEAD_TEXT), NTADR_A(12, 14));

      set_scroll_x(0);

      music_stop();
    }
  }
}

void load_title(void) {
  pal_bg(palette_title);
  pal_spr(palette_sp);
  vram_adr(NAMETABLE_A);
  vram_unrle(title);
  game_mode = MODE_TITLE;
}

void load_room(void) {
  char offset = Level_offsets[level];

  set_data_pointer(Levels_list[offset]);
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
  set_data_pointer(Levels_list[offset + 1]);
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
  memcpy(c_map, Levels_list[offset], 240);

  sprite_obj_init();

  BoxGuy1.x = 0x4000;
  BoxGuy1.y = 0xc400;
  BoxGuy1.vel_x = 0;
  BoxGuy1.vel_y = 0;

  map_loaded = 0;
}

void draw_sprites(void) {
  // clear all sprites from sprite buffer
  oam_clear();

  char x = high_byte(BoxGuy1.x);
  if (x > 0xfc || !x)
    x = 1;
  // draw 1 hero
  oam_meta_spr(x, high_byte(BoxGuy1.y),
               direction == LEFT ? RoundSprL : RoundSprR);

  // draw coin sprites
  for (char i = 0; i < MAX_COINS; ++i) {
    char y = coin_y[i];
    if (y == TURN_OFF)
      continue;
    if (get_frame_count() & 8)
      ++y; // bounce the coin
    if (!coin_active[i] || coin_x[i] > 0xf0)
      continue;
    if (y < 0xf0)
      oam_meta_spr(coin_x[i], y,
                   coin_type[i] == COIN_REG ? CoinSpr : BigCoinSpr);
  }

  // draw enemy sprites
  // for shuffling 16 enemies
  static const char shuffle_array[] = {
      0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
      15, 14, 13, 12, 11, 10, 9,  8,  7,  6,  5,  4,  3,  2,  1,  0,
      0,  2,  4,  6,  8,  10, 12, 14, 1,  3,  5,  7,  9,  11, 13, 15,
      15, 13, 11, 9,  7,  5,  3,  1,  14, 12, 10, 8,  6,  4,  2,  0};
  char offset =
      (get_frame_count() & 3) * 16 /* the size of the shuffle array */;
  for (char i = 0; i < MAX_ENEMY; ++i) {
    char j = shuffle_array[offset];
    ++offset;
    if (enemy_y[j] == TURN_OFF)
      continue;
    if (!enemy_active[j])
      continue;
    char x = enemy_x[j];
    if (!x)
      x = 1; // problems with x = 0
    if (x > 0xf0)
      continue;
    if (enemy_y[j] < 0xf0)
      oam_meta_spr(x, enemy_y[j], enemy_anim[j]);
  }

  // last, draw coin in upper left
  oam_meta_spr(0x10, 0x0f, CoinHud);
  oam_spr(0x20, 0x17, (coins / 10) + 0xf0, 1);
  oam_spr(0x28, 0x17, (coins % 10) + 0xf0, 1);
}

void movement(void) {
  static char short_jump_count = 0;

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
      sfx_play(SFX_JUMP, 0);
      short_jump_count = 1;
    }
  }

  // allow shorter jumps
  if (short_jump_count)
    if (++short_jump_count > 30)
      short_jump_count = 0;
  if (short_jump_count && ((pad1 & PAD_A) == 0) && (BoxGuy1.vel_y < -0x200)) {
    BoxGuy1.vel_y = -0x200;
    short_jump_count = 0;
  }

  // do we need to load a new collision map? (scrolled into a new room)
  if ((scroll_x & 0xff) < 4) {
    if (!map_loaded) {
      new_cmap();
      map_loaded = 1; // only do once
    }
  } else {
    map_loaded = 0;
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

void check_spr_objects(void) {
  ++enemy_frames;
  // mark each object "active" if they are, and get the screen x

  for (char i = 0; i < MAX_COINS; ++i) {
    coin_active[i] = 0; // default to zero
    if (coin_y[i] != TURN_OFF) {
      unsigned x = (coin_room[i] << 8 | coin_actual_x[i]) - scroll_x;
      coin_active[i] = !high_byte(x);
      coin_x[i] = x & 0xff; // screen x coords
    }
  }

  for (char i = 0; i < MAX_ENEMY; ++i) {
    enemy_active[i] = 0; // default to zero
    if (enemy_y[i] != TURN_OFF) {
      unsigned x = (enemy_room[i] << 8 | enemy_actual_x[i]) - scroll_x;
      enemy_active[i] = !high_byte(x);
      if (!enemy_active[i])
        continue;
      enemy_x[i] = x & 0xff; // screen x coords

      enemy_moves(i); // if active, do it's moves now
    }
  }
}

void enemy_moves(char index) {
  if (enemy_type[index] == ENEMY_CHASE) {
    // for bg collisions
    char x = enemy_x[index];
    char y = enemy_y[index] + 6; // mid point
    char width = 13;

    enemy_anim[index] = EnemyChaseSpr;
    if (enemy_frames & 1)
      return; // half speed
    if (enemy_x[index] > high_byte(BoxGuy1.x)) {
      bg_collision_fast(x - 1, y, width);
      if (collision_L)
        return;
      if (enemy_actual_x[index] == 0)
        --enemy_room[index];
      --enemy_actual_x[index];
    } else if (enemy_x[index] < high_byte(BoxGuy1.x)) {
      bg_collision_fast(x + 1, y, width);
      if (collision_R)
        return;
      ++enemy_actual_x[index];
      if (enemy_actual_x[index] == 0)
        ++enemy_room[index];
    }
  } else if (enemy_type[index] == ENEMY_BOUNCE) {
    char anim_frame = (enemy_frames + (index << 3)) & 0x3f;
    if (anim_frame < 16) {
      enemy_anim[index] = EnemyBounceSpr;
    } else if (anim_frame < 40) {
      --enemy_y[index];
      enemy_anim[index] = EnemyBounceSpr2;
    } else {
      enemy_anim[index] = EnemyBounceSpr2;
      // check ground collision
      bg_check_low(enemy_x[index], enemy_y[index] - 1, 15, 15);
      if (!collision_D)
        ++enemy_y[index];
    }
  }
}

void bg_collision_fast(char x, char y, char width) {
  // rewrote this for enemies, bg_collision was too slow
  collision_L = 0;
  collision_R = 0;

  if (y >= 0xf0)
    return;

  unsigned upper_left = x + scroll_x; // upper left (temp6 = save for reuse)

  if (bg_collision_sub(upper_left, y) &
      COL_ALL) // find a corner in the collision map
    ++collision_L;

  // upper right
  unsigned upper_right = upper_left + width;

  if (bg_collision_sub(upper_right, y) &
      COL_ALL) // find a corner in the collision map
    ++collision_R;
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
                       y_top) &
      COL_ALL) { // find a corner in the collision map
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
  return is_solid[typ];
}

void draw_screen_R(void) {
  // scrolling to the right, draw metatiles as we go
  unsigned pseudo_scroll_x = scroll_x + 0x120;

  char room = pseudo_scroll_x >> 8;

  set_data_pointer(Levels_list[Level_offsets[level] + room]);
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
  memcpy(map, Levels_list[Level_offsets[level] + room], 240);
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

void sprite_collisions(void) {
  struct Box {
    char x, y, width, height;
  };

  struct Box boxguy1_box = {high_byte(BoxGuy1.x), high_byte(BoxGuy1.y),
                            HERO_WIDTH, HERO_HEIGHT};
  struct Box other_box;

  for (char i = 0; i < MAX_COINS; ++i) {
    if (!coin_active[i])
      continue;

    if (coin_type[i] == COIN_REG) {
      other_box.width = COIN_WIDTH;
      other_box.height = COIN_HEIGHT;
    } else {
      other_box.width = BIG_COIN;
      other_box.height = BIG_COIN;
    }

    other_box.x = coin_x[i];
    other_box.y = coin_y[i];
    if (!check_collision(&boxguy1_box, &other_box))
      continue;

    coin_y[i] = TURN_OFF;
    sfx_play(SFX_DING, 0);
    ++coins;

    if (coin_type[i] == COIN_END)
      ++level_up;
  }

  other_box.width = ENEMY_WIDTH;
  other_box.height = ENEMY_HEIGHT;
  for (char i = 0; i < MAX_ENEMY; ++i) {
    if (!enemy_active[i])
      continue;

    other_box.x = enemy_x[i];
    other_box.y = enemy_y[i];
    if (!check_collision(&boxguy1_box, &other_box))
      continue;

    enemy_y[i] = TURN_OFF;
    enemy_active[i] = 0;
    sfx_play(SFX_NOISE, 0);
    if (coins) {
      coins -= 5;
      if (coins > 0x80)
        coins = 0;
    } else { // die
      ++death;
    }
  }
}

void sprite_obj_init(void) {
  const unsigned char *coins = Coins_list[level];
  char i, j;
  for (i = 0, j = 0; i < MAX_COINS; ++i) {
    coin_x[i] = 0;
    coin_y[i] = coins[j];
    if (coin_y[i] == TURN_OFF)
      break;
    coin_active[i] = 0;
    coin_room[i] = coins[++j];
    coin_actual_x[i] = coins[++j];
    coin_type[i] = coins[++j];
    ++j;
  }
  for (++i; i < MAX_COINS; ++i)
    coin_y[i] = TURN_OFF;

  const unsigned char *enemies = Enemy_list[level];
  for (i = 0, j = 0; i < MAX_ENEMY; ++i) {
    enemy_x[i] = 0;
    enemy_y[i] = enemies[j];
    if (enemy_y[i] == TURN_OFF)
      break;
    enemy_active[i] = 0;
    enemy_room[i] = enemies[++j];
    enemy_actual_x[i] = enemies[++j];
    enemy_type[i] = enemies[++j];
    ++j;
  }
  for (++i; i < MAX_ENEMY; ++i)
    enemy_y[i] = TURN_OFF;
}
