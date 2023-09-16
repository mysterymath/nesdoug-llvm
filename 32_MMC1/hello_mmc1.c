/*	example of MMC1 for llvm-mos
 *	Doug Fraker 2019, 2022
 */

#include <bank.h>
#include <nesdoug.h>
#include <neslib.h>

asm(".globl __prg_ram_size;\n__prg_ram_size = 8\n");

enum { BANK_0, BANK_1, BANK_2, BANK_3, BANK_4, BANK_5, BANK_6 };
// 7 shouldn't be needed, that's the fixed bank, just call it normally

static char arg1;
static char arg2;

// extra RAM at $6000-$7fff
__attribute__((section(".prg_ram_0.noinit"))) char wram_array[0x2000];

#define BLACK 0x0f
#define DK_GY 0x00
#define LT_GY 0x10
#define WHITE 0x30

// test putting things in other banks
#pragma clang section text = ".prg_rom_0.text" rodata = ".prg_rom_0.rodata"

static const char TEXT0[] = "BANK0";

__attribute__((noinline)) void function_bank0(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 4));
  vram_write(TEXT0, sizeof(TEXT0));
  ppu_on_all();
}

#pragma clang section text = ".prg_rom_1.text" rodata = ".prg_rom_1.rodata"
void function_bank2(void); // prototype

__attribute__((noinline)) void function_bank1(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 6));

  static const char TEXT1[] = "BANK1";
  vram_write(TEXT1, sizeof(TEXT1));
  ppu_on_all();

  banked_call(BANK_2, function_bank2);
}

#pragma clang section text = ".prg_rom_2.text" rodata = ".prg_rom_2.rodata"

__attribute__((noinline)) void function_bank2(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 8));
  static const char TEXT2[] = "BANK2";
  vram_write(TEXT2, sizeof(TEXT2));
  ppu_on_all();
}

#pragma clang section text = ".prg_rom_3.text" rodata = ".prg_rom_3.rodata"

__attribute__((noinline)) void function_bank3(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 10));
  static const char TEXT3[] = "BANK3";
  vram_write(TEXT3, sizeof(TEXT3));

  vram_put(0);
  vram_put(arg1); // these args were passed via globals
  vram_put(arg2);
  ppu_on_all();
}

#pragma clang section text = ".prg_rom_4.text" rodata = ".prg_rom_4.rodata"

__attribute__((noinline)) void function_bank4(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 12));
  static const char TEXT4[] = "BANK4";
  vram_write(TEXT4, sizeof(TEXT4));
  ppu_on_all();
}

#pragma clang section text = ".prg_rom_5.text" rodata = ".prg_rom_5.rodata"

__attribute__((noinline)) void function_2_bank5(void) {
  vram_adr(NTADR_A(8, 14));
  static const char TEXT5B[] = "ALSO THIS";
  vram_write(TEXT5B, sizeof(TEXT5B));
}

__attribute__((noinline)) void function_bank5(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 14));
  static const char TEXT5[] = "BANK5";
  vram_write(TEXT5, sizeof(TEXT5));

  // use a regular function call to call a function in
  // the same bank
  function_2_bank5();

  ppu_on_all();
}

#pragma clang section text = ".prg_rom_6.text" rodata = ".prg_rom_6.rodata"

__attribute__((noinline)) void function_bank6(void) {
  ppu_off();
  vram_adr(NTADR_A(1, 16));
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
  music_init(music_data);
  sounds_init(sounds_data);

  set_mmc1_ctrl(0x1f);
  set_prg_bank(0);
  set_chr_bank_0(0);
  set_chr_bank_1(1);

  char song = 0;
  music_play(song);

  wram_array[0] = 'A'; // put some values at $6000-7fff
  wram_array[2] = 'C'; // for later testing

  draw_sprites();

  ppu_off(); // screen off
  static const char palette[16] = {
      BLACK,
      DK_GY,
      LT_GY,
      WHITE,
  };
  pal_bg(palette);  //	load the BG palette
  pal_spr(palette); // load the sprite palette
  ppu_on_all();     // turn on screen

  // calling functions in other banks

  banked_call(BANK_0, function_bank0);
  banked_call(BANK_1, function_bank1);
  // banked_call(BANK_2, function_bank2); // moved to bank 1

  arg1 = 'G'; // must pass arguments with globals
  arg2 = '4';
  banked_call(BANK_3, function_bank3);
  banked_call(BANK_4, function_bank4);
  banked_call(BANK_5, function_bank5);
  banked_call(BANK_6, function_bank6);

  ppu_off(); // screen off
  vram_adr(NTADR_A(1, 18));
  static const char text[] = "BACK IN FIXED BANK, 7";
  vram_write(text, sizeof(text)); // code running from the fixed bank

  // set the bank 0 at $8000-bfff
  // with a swappable bank in place, we can read it from the fixed bank
  set_prg_bank(0);
  vram_adr(NTADR_A(1, 20));
  vram_write(TEXT0, sizeof(TEXT0)); // this array is in bank 0

  ppu_on_all(); //	turn on screen

  char pad1, pad1_new;
  char char_state = 0;
  char sound = 0;
  char pauze = 0;
  while (1) { // infinite loop
    ppu_wait_nmi();

    pad1 = pad_poll(0);
    pad1_new = get_pad_new(0);

    if (pad1_new & PAD_START) {
      char_state = (char_state + 1) & 3; // keep 0-3

      defer_chr_bank_0(char_state); // switch the BG bank
      // note just the tileset #0 is changed,
      // the sprite bank would have to be changed with
      // set_chr_bank_1();

      sample_play(1);
    }

    if (pad1_new & PAD_A) {
      song = (song + 1) & 1; // keep 0 or 1
      music_play(song);
    }

    if (pad1_new & PAD_B) {
      sound = (sound + 1) & 1; // keep 0 or 1
      sfx_play(sound, 0);
    }

    if (pad1_new & PAD_SELECT) {
      pauze = (pauze + 1) & 1; // keep 0 or 1
      music_pause(pauze);
    }
  }
}

// testing sprites using the second tileset
void draw_sprites(void) {
  bank_spr(1);
  oam_clear();
  oam_spr(0x50, 0x10, 0, 0);
  oam_spr(0x58, 0x10, 1, 0);
  oam_spr(0x50, 0x18, 0x10, 0);
  oam_spr(0x58, 0x18, 0x11, 0);
}
