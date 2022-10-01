/*	example code for cc65, for NES
 *  testing the zapper gun on controller slot 2
 *	using neslib
 *	Doug Fraker 2018
 */

#include <nesdoug.h>
#include <neslib.h>
#include <zaplib.h>

#include "NES_ST/Zap_Test.h"
#include "Sprites.h"

char score1000;
char score100;
char score10;
char score1;

char star_active;
char star_color;
unsigned star_x;
unsigned star_y;
unsigned star_x_speed;
unsigned star_y_speed;
char star_wait;

// PROTOTYPES
void move_star(void);
void adjust_score(void);
void new_star(void);
void draw_star(void);

const char pal1[16] = {
    // clang-format off
    0x0f, 0x00, 0x10, 0x30,
    // clang-format on
};

const char pal2[] = {
    // clang-format off
    0x0f, 0x12, 0x22, 0x30,
    0x0f, 0x15, 0x25, 0x30,
    0x0f, 0x17, 0x27, 0x30,
    0x0f, 0x19, 0x29, 0x30,
    // clang-format on
};

int main(void) {

  ppu_off(); // screen off

  pal_bg(pal1);  //	load the palette
  pal_spr(pal2); //	load the palette

  bank_spr(1); // sprites use the 2nd tileset

  vram_adr(NAMETABLE_A);
  // this sets a start position on the BG, top left of screen
  // vram_adr() and vram_unrle() need to be done with the screen OFF

  vram_unrle(Zap_Test);
  // this unpacks an rle compressed full nametable
  // created by NES Screen Tool

  ppu_wait_nmi(); // wait

  //	music_play(0); // silence

  set_vram_buffer(); // points ppu update to vram_buffer, do this at least once

  ppu_on_all(); // turn on screen

  char pad2_zapper = 0;
  while (1) {
    // infinite loop
    ppu_wait_nmi(); // wait till beginning of the frame

    oam_clear();

    char zapper_ready =
        pad2_zapper ^ 1; // XOR last frame, make sure not held down still

    // is trigger pulled?
    pad2_zapper = zap_shoot(1); // controller slot 2

    if (star_active) {
      move_star();
      draw_star();
      if (pad2_zapper && zapper_ready) {

        // trigger pulled, play bang sound
        sfx_play(0, 0);

        // bg off, project white boxes
        oam_clear();
        oam_meta_spr(high_byte(star_x), high_byte(star_y), WhiteBox);
        ppu_mask(0x16); // BG off, won't happen till NEXT frame

        ppu_wait_nmi(); // wait till the top of the next frame
        // this frame will display no BG and a white box

        oam_clear();    // clear the NEXT frame
        draw_star();    // draw a star on the NEXT frame
        ppu_mask(0x1e); // bg on, won't happen till NEXT frame

        char hit_detected = zap_read(1); // look for light in zapper, port 2
        if (hit_detected) {
          ++score1;
          adjust_score();
          // delete object, set wait
          star_active = 0;
          star_wait = 20; // to time the next spawn
        }
        // if hit failed, it should have already ran into the next nmi
      }
    } else if (star_wait) {
      --star_wait;
    } else {
      new_star();
    }
  }
}

void move_star(void) {
  // gravity, star_y_speed = 16 bit, upper 8 bits = pixel, lower = subpixel
  star_y_speed += 0x0010;
  if (star_y_speed < 0x8000 && star_y_speed > 0x0400)
    star_y_speed = 0x0400;

  star_x += star_x_speed;
  if (star_x >= 0xf000)
    star_active = 0;

  star_y += star_y_speed;
  if (star_y >= 0xe000)
    star_active = 0;
}

void adjust_score(void) {
  if (score1 >= 10) {
    score1 = 0;
    ++score10;
    if (score10 >= 10) {
      score10 = 0;
      ++score100;
      if (score100 >= 10) {
        score100 = 0;
        ++score1000;
      }
    }
  }

  if (score1000 >= 10) { // maximum 9999
    score1000 = 9;
    score100 = 9;
    score10 = 9;
    score1 = 9;
  }

  // copy score to BG
  one_vram_buffer(score1000 + 0x30, NTADR_A(9, 4));
  one_vram_buffer(score100 + 0x30, NTADR_A(10, 4));
  one_vram_buffer(score10 + 0x30, NTADR_A(11, 4));
  one_vram_buffer(score1 + 0x30, NTADR_A(12, 4));
}

void new_star(void) {
  star_active = 1;
  star_color = (star_color + 1) & 1; // 0 or 1
  star_x = (rand8() << 7) + 0x4000;  // should give 0x4000-0xbf80
  star_y = 0xd000;                   // int
  star_x_speed = ((rand8() & 0x1f) - 0x0f) << 4;
  star_y_speed = 0xfc00;
}

void draw_star(void) {
  oam_meta_spr(high_byte(star_x), high_byte(star_y),
               star_color ? StarLight : StarDark);
}
