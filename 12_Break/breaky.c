/*	example code for llvm-mos, for NES
 *  a break the blocks clone, simplified
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>
#include <string.h>

#include "BG/breaky_bg2.h"
#include "CSV/c1.h"
#include "Sprites.h" // holds our metasprite data

char c_map[256];
// collision map
// 16 wide has easier math than 14 wide, so sides are padded with 0

char pad1;
char pad1_new;
char lives01 = 3;
char score10;
char score01;
char ball_state;
enum { BALL_OFF, BALL_STUCK, BALL_ACTIVE };
unsigned char ball_x_rel; // if stuck
unsigned char ball_count; // if off
unsigned char ball_direction;
enum { GOING_UP, GOING_DOWN };

#define PADDLE_MIN 0x10
#define PADDLE_MAX 0xd0
#define PADDLE_Y 0xd0
#define MAX_UP 0x30
#define MAX_DOWN 0xe0
#define BALL_MIN 0x10
#define BALL_MAX 0xea

struct SpObject {
  unsigned char X;
  unsigned char Y;
  unsigned char width;
  unsigned char height;
};

struct SpObject Paddle = {0x75, PADDLE_Y, 0x1f, 7};
struct SpObject Ball = {0xff, 0xff, 5, 5}; // balls x and x will be init later

void draw_bg(void);
void draw_sprites(void);
void movement(void);
void score_lives_draw(void);
void adjust_score(void);
void hit_block(char x, char y, char offset);

int main(void) {

  ppu_off(); // screen off

  // load the palettes
  static const unsigned char palette_bg[16] = {
      0x0f, 0x00, 0x10, 0x30, 0x0f, 0x01, 0x21, 0x39,
      0x0f, 0x04, 0x24, 0x36, 0x0f, 0x09, 0x29, 0x38};
  pal_bg(palette_bg);
  static const unsigned char palette_sp[16] = {
      0x0f, 0x00, 0x10, 0x30, // greys
  };
  pal_spr(palette_sp);

  // use the second set of tiles for sprites
  // both bg and sprites are set to 0 by default
  bank_spr(1);

  set_scroll_y(0xff); // shift the bg down 1 pixel

  draw_bg();

  set_vram_buffer(); // do at least once, sets a pointer to a buffer

  // turn on screen
  // ppu_on_all(); // already done in draw_bg()

  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame

    pad1 = pad_poll(0);        // read the first controller
    pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first

    score_lives_draw();

    if (lives01) {
      movement();
      draw_sprites();
    } else {
      oam_clear(); // game over, just turn everything off
    }
  }
}

void draw_bg(void) {
  ppu_off(); // screen off

  vram_adr(NAMETABLE_A);
  // this sets a start position on the BG, top left of screen

  vram_unrle(breaky_bg2);
  // this unpacks a compressed full nametable

  memcpy(c_map, c1, 256);
  // copy the collision map to c_map

  vram_adr(NTADR_A(0, 6));
  // sets a start address, skipping the top of the screen

  for (char y = 0; y < 16; ++y) {
    for (char x = 0; x < 16; ++x) {
      if ((x == 0) || (x == 15)) {
        vram_put(0x10); // wall at the edges
        vram_put(0x10);
      } else {
        if (c_map[(y << 4) + x]) { // if block = yes
          vram_put(0x11);          // draw block
          vram_put(0x12);
        } else {
          vram_put(0); // else draw blank
          vram_put(0);
        }
      }
    }
  }

  ppu_on_all();
}

void draw_sprites(void) {
  // clear all sprites from sprite buffer
  oam_clear();

  // draw 2 metasprites
  oam_meta_spr(Paddle.X, Paddle.Y, PaddleSpr);
  oam_meta_spr(Ball.X, Ball.Y, BallSpr);
}

void movement(void) {
  // paddle move
  if (pad1 & PAD_LEFT) {
    Paddle.X -= 2;
    if (Paddle.X < PADDLE_MIN)
      Paddle.X = PADDLE_MIN;
  }
  if (pad1 & PAD_RIGHT) {
    Paddle.X += 2;
    if (Paddle.X > PADDLE_MAX)
      Paddle.X = PADDLE_MAX;
  }

  switch (ball_state) {
  case BALL_OFF: // ball is inactive, wait a second
    ++ball_count;
    if (ball_count > 60) {
      ball_state = BALL_STUCK;
      ball_x_rel = 0x0d;
      ball_count = 0;
      return;
    }
    Ball.Y = 0xff; // off screen
  case BALL_STUCK:
    Ball.X = Paddle.X + ball_x_rel;
    Ball.Y = Paddle.Y - 4;

    if (pad1_new & (PAD_A | PAD_B)) { // any new a or b press to start
      ball_state = BALL_ACTIVE;
      ball_direction = GOING_UP;
      if (Ball.X < BALL_MIN)
        Ball.X = BALL_MIN;
      if (Ball.X > BALL_MAX)
        Ball.X = BALL_MAX;
      return;
    }
    break;
  case BALL_ACTIVE: {
    if (ball_direction == GOING_UP) {
      Ball.Y -= 3;
      if (Ball.Y < MAX_UP) {
        ball_direction = GOING_DOWN;
      }
    } else { // going down
      Ball.Y += 3;
      if (Ball.Y > MAX_DOWN) {
        --lives01;
        ball_state = BALL_OFF;
      }

      // collision w paddle = stuck
      if (check_collision(&Ball, &Paddle)) {
        ball_state = BALL_STUCK;
        ball_x_rel = Ball.X - Paddle.X;
      }
    }

    // collision w blocks

    char x = (Ball.X + 1) & 0xf0; // tiles are 16 px wide
    char y = (Ball.Y + 2) & 0xf8; // tiles only 8 px high
    if (y < 0xaf) { // Y of 0x30 + 16*8 = b0. Ball.Y>b0 = off the c_map

      char offset = (x >> 4) + (((y - 0x30) << 1) & 0xf0);
      // << 1 because tiles only 8 px high

      if (c_map[offset]) { // hit a block
        hit_block(x, y, offset);
        return;
      }
    }

    // check a little more to the right
    x = (Ball.X + 4) & 0xf0; // tiles are 16 px wide
    y = (Ball.Y + 2) & 0xf8; // tiles only 8 px high
    if (y < 0xaf) {          // Y of 0x30 + 16*8 = b0. Ball.Y>b0 = off the c_map

      char offset = (x >> 4) + (((y - 0x30) << 1) & 0xf0);
      // << 1 because tiles only 8 px high

      if (c_map[offset]) { // hit a block
        hit_block(x, y, offset);
      }
    }
    break;
  }
  }
}

void hit_block(char x, char y, char offset) {
  score01 += 1;
  adjust_score();
  ball_direction = GOING_DOWN;
  Ball.Y += 3;
  c_map[offset] = 0;

  unsigned address = get_ppu_addr(0, x, y);
  address = address & 0xfffe;  // start with the left tile
  one_vram_buffer(0, address); // tile 0 = blank
  ++address;
  one_vram_buffer(0, address); // also the one to the right of it
}

void score_lives_draw(void) {
  one_vram_buffer(score10 + '0', NTADR_A(10, 3));
  one_vram_buffer(score01 + '0', NTADR_A(11, 3));
  one_vram_buffer('0', NTADR_A(26, 3));
  one_vram_buffer(lives01 + '0', NTADR_A(27, 3));
}

void adjust_score(void) {
  if (score01 >= 10) {
    score01 -= 10;
    ++score10;
  }
}
