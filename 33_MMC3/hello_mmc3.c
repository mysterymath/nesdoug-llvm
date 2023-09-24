/*	example of MMC3 for llvm-mos
 *	Doug Fraker 2019
 */

#include <6502.h>
#include <nesdoug.h>
#include <neslib.h>
#include <mapper.h>
#include <string.h>

asm (".globl __prg_ram_size\n__prg_ram_size=8\n");

// a 16x16 pixel metasprite

const char RoundSprL[] = {
    // clang-format off
  0xff, 0xff, 0x02, 0,
  7,    0xff, 0x03, 0,
  0xff, 7,    0x12, 0,
  7,    7,    0x13, 0,
  128
    // clang-format on
};

const char RoundSprR[] = {
    // clang-format off
  0xff, 0xff, 0x00, 0,
  7,    0xff, 0x01, 0,
  0xff, 7,    0x10, 0,
  7,    7,    0x11, 0,
  128
    // clang-format on
};

// GLOBAL VARIABLES

char arg1;
char arg2;

char sprite_x;
char sprite_y;
char dirLR;

char irq_array[32];
char double_buffer[32];

// extra RAM at $6000-$7fff
__attribute__((section(".prg_ram"))) char wram_array[0x2000];

// test putting things in other banks
#pragma clang section text = ".prg_rom_0.text" rodata = ".prg_rom_0.rodata"
void function_bank0(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 4));
  static const char TEXT0[] = "BANK0";
  vram_write(TEXT0, sizeof(TEXT0));
  ppu_on_all();
}

#pragma clang section text = ".prg_rom_1.text" rodata = ".prg_rom_1.rodata"

void function_bank2(void); // a prototype can be anywhere
                           // as long as it's above the code that uses it

void function_bank1(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 6));
  static const char TEXT1[] = "BANK1";
  vram_write(TEXT1, sizeof(TEXT1));
  ppu_on_all();

  banked_call(2, function_bank2);
  // calling a function in another bank, use banked_call()
}

#pragma clang section text = ".prg_rom_2.text" rodata = ".prg_rom_2.rodata"

void function_same_bank() {
  vram_put(0);
  vram_put('H');
  vram_put('I');
}

void function_bank2(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 8));
  static const char TEXT2[] = "BANK2";
  vram_write(TEXT2, sizeof(TEXT2));

  function_same_bank();
  // calling a function in same bank, use regular function calls

  ppu_on_all();
}

#pragma clang section text = ".prg_rom_3.text" rodata = ".prg_rom_3.rodata"

void function_bank3(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 10));
  static const char TEXT3[] = "BANK3";
  vram_write(TEXT3, sizeof(TEXT3));

  vram_put(0);
  vram_put(arg1); // these args were passed via globals
  vram_put(arg2);
  ppu_on_all();
}

#pragma clang section text = ".prg_rom_6.text" rodata = ".prg_rom_6.rodata"

void function_bank6(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 14));
  static const char TEXT6[] = "BANK6";
  vram_write(TEXT6, sizeof(TEXT6));

  vram_put(0);
  vram_put(wram_array[0]); // testing the $6000-7fff area
  vram_put(wram_array[2]); // should print A, C
  ppu_on_all();
}

// the fixed bank, bank 7
#pragma clang section text = "" rodata = ""

void draw_sprites(void);

extern const char music_data[];
extern const char sounds_data[];
int main(void) {
  disable_irq();
  CLI();
  set_prg_mode(PRG_MODE_0);
  set_chr_a12_inversion(CHR_A12_INVERT);
  set_prg_8000(0);
  set_chr_bank(0, 4);
  set_chr_bank(1, 6);
  set_chr_bank(2, 0);
  set_chr_bank(3, 1);
  set_chr_bank(4, 2);
  set_chr_bank(5, 3);
  set_wram_mode(WRAM_ON);

  banked_music_init(12, music_data);
  banked_sounds_init(12, sounds_data);

  set_mirroring(MIRROR_HORIZONTAL);
  bank_spr(1);
  irq_array[0] = 0xff;    // end of data
  set_irq_ptr(irq_array); // point to this array

  // clear the WRAM, not done by the init code
  memset(wram_array, 0, 0x2000);

  wram_array[0] = 'A'; // put some values at $6000-7fff
  wram_array[2] = 'C'; // for later testing

  ppu_off(); // screen off

  static const char palette_bg[] = {
      // clang-format off
      0x0f, 0, 0x10, 0x30,
      0x0f, 0, 0,    0,
      0x0f, 0, 0,    0,
      0x0f, 0, 0,    0
      // clang-format on
  };
  pal_bg(palette_bg); //	load the BG palette

  static const char palette_spr[] = {
      // clang-format off
      0x0f, 0x09, 0x19, 0x29, // greens
      0x0f, 0,    0,    0,
      0x0f, 0,    0,    0,
      0x0f, 0,    0,    0
      // clang-format on
  };
  pal_spr(palette_spr); // load the sprite palette

  // draw some things
  vram_adr(NTADR_A(20, 3)); // gear and squares
  vram_put(0xc0);
  vram_put(0xc1);
  vram_put(0xc2);
  vram_put(0xc3);
  vram_adr(NTADR_A(20, 4));
  vram_put(0xd0);
  vram_put(0xd1);
  vram_put(0xd2);
  vram_put(0xd3);
  vram_adr(NTADR_A(20, 7));
  vram_put(0xc0);
  vram_put(0xc1);
  vram_put(0xc2);
  vram_put(0xc3);
  vram_adr(NTADR_A(20, 8));
  vram_put(0xd0);
  vram_put(0xd1);
  vram_put(0xd2);
  vram_put(0xd3);

  vram_adr(NTADR_A(20, 5)); // blocks of color
  vram_put(0x2);
  vram_put(0x2);
  vram_put(0x2);
  vram_put(0x2);
  vram_adr(NTADR_A(20, 9));
  vram_put(0x2);
  vram_put(0x2);
  vram_put(0x2);
  vram_put(0x2);
  vram_adr(NTADR_A(20, 13));
  vram_put(0x2);
  vram_put(0x2);
  vram_put(0x2);
  vram_put(0x2);

  music_play(0);
  // ppu_on_all(); // turn on screen

  set_chr_mode_5(8); // make sure the gear tiles loaded
                     // for the first few frames

  // calling functions in other banks

  banked_call(0, function_bank0);
  banked_call(1, function_bank1);
  // banked_call(2, function_bank2); // moved to bank 1

  arg1 = 'G'; // must pass arguments with globals
  arg2 = '4';
  banked_call(3, function_bank3);
  banked_call(6, function_bank6);

  ppu_off(); // screen off
  vram_adr(NTADR_A(1, 16));
  static const char text[] = "BACK IN FIXED BANK";
  vram_write(text, sizeof(text));

  sprite_x = 0x50;
  sprite_y = 0x30;
  draw_sprites();

  ppu_on_all(); //	turn on screen

  char char_state = 0;
  // fixed point 8.8
  unsigned scroll_top;
  unsigned scroll2;
  unsigned scroll3;
  unsigned scroll4;
  while (1) { // infinite loop
    ppu_wait_nmi();

    char pad1 = pad_poll(0);
    char pad1_new = get_pad_new(0);

    if (pad1 & PAD_A) {   // shift screen right = subtract from scroll
      scroll_top -= 0x80; // sub pixel movement
      scroll2 -= 0x100;   // 1 pixel
      scroll3 -= 0x180;
      scroll4 -= 0x200;
    }

    if (pad1 & PAD_B) {   // shift screen right = subtract from scroll
      scroll_top += 0x80; // sub pixel movement
      scroll2 += 0x100;   // 1 pixel
      scroll3 += 0x180;
      scroll4 += 0x200;
    }

    set_scroll_x(scroll_top >> 8);

    if ((get_frame_count() & 0x03) == 0) { // every 4th frame
      ++char_state;
      if (char_state >= 4)
        char_state = 0;
    }

    // load the irq array with values it can parse
    // ! CHANGED it, double buffered so we aren't editing the same
    // array that the irq system is reading from

    // top of the screen, probably read in v-blank
    // put whole screen things here, before the split
    double_buffer[0] = 0xfc; // CHR mode 5, change the 0xc00-0xfff tiles
    double_buffer[1] = 8;    // top of the screen, static value
    double_buffer[2] = 47;   // value < 0xf0 = scanline count, 1 less than #

    // after the first split
    double_buffer[3] = 0xf5; // H scroll change, do first for timing
    double_buffer[4] = scroll2 >> 8; // scroll value
    double_buffer[5] = 0xfc; // CHR mode 5, change the 0xc00-0xfff tiles
    double_buffer[6] = 8 + char_state; // value = 8,9,10,or 11
    double_buffer[7] = 29;             // scanline count

    // after the 2nd split
    double_buffer[8] = 0xf5; // H scroll change
    double_buffer[9] = scroll3 >> 8;  // scroll value
    double_buffer[10] = 0xf1; // $2001 test changing color emphasis
    double_buffer[11] = 0xfe; // value COL_EMP_DARK 0xe0 + 0x1e
    // double_buffer[11] = 0x1f; // alternate value for grayscale
    double_buffer[12] = 30; // scanline count

    // after the 3rd split
    double_buffer[13] = 0xf5; // H scroll change
    double_buffer[14] = scroll4 >> 8; // scroll value
    double_buffer[15] = 30;   // scanline count

    double_buffer[16] = 0xf5; // H scroll change
    double_buffer[17] = 0;    // to zero, to set the fine X scroll
    double_buffer[18] = 0xf6; // 2 writes to 2006 shifts screen
    double_buffer[19] = 0x20; // need 2 values...
    double_buffer[20] = 0x00; // PPU address $2000 = top of screen

    double_buffer[21] = 0xff; // end of data

    if (pad1 & PAD_LEFT) { // shift screen right = subtract from scroll
      sprite_x -= 1;
      dirLR = 0;
    } else if (pad1 & PAD_RIGHT) { // shift screen right = subtract from scroll
      sprite_x += 1;
      dirLR = 1;
    }
    if (pad1 & PAD_UP) { // shift screen right = subtract from scroll
      sprite_y -= 1;
    } else if (pad1 & PAD_DOWN) { // shift screen right = subtract from scroll
      sprite_y += 1;
    }

    draw_sprites();

    // wait till the irq system is done before changing it
    // this could waste a lot of CPU time, so we do it last
    while (!is_irq_done()) { // have we reached the 0xff, end of data
                             // is_irq_done() returns zero if not done
                             // do nothing while we wait
    }

    // copy from double_buffer to the irq_array
    memcpy(irq_array, double_buffer, sizeof(irq_array));
  }
}

// testing sprites using the second tileset
void draw_sprites(void) {
  oam_clear();
  if (!dirLR) { // left
    oam_meta_spr(sprite_x, sprite_y, RoundSprL);
  } else {
    oam_meta_spr(sprite_x, sprite_y, RoundSprR);
  }
}
