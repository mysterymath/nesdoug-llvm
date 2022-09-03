/*	simple Hello World, for llvm-mos, for NES
 *  writing to the screen with rendering ON
 *	using neslib
 *	Doug Fraker 2018
 */

#include <neslib.h>

#define BLACK 0x0f
#define DK_GY 0x00
#define LT_GY 0x10
#define WHITE 0x30
// there's some oddities in the palette code, black must be 0x0f, white must be
// 0x30

int main(void) {
  const char palette[16] = {BLACK, DK_GY, LT_GY, WHITE};

  // example of sequential vram data
  const char text[] = {MSB(NTADR_A(10, 14)) |
                           NT_UPD_HORZ, // where to write, repeat horizontally
                       LSB(NTADR_A(10, 14)),
                       12,  // length of write
                       'H', // the data to be written, 12 chars
                       'E',
                       'L',
                       'L',
                       'O',
                       ' ',
                       'W',
                       'O',
                       'R',
                       'L',
                       'D',
                       '!',
                       NT_UPD_EOF}; // data must end in EOF

  // example of non-sequential vram data
  const char two_letters[] = {MSB(NTADR_A(8, 17)), LSB(NTADR_A(8, 17)), 'A',
                              MSB(NTADR_A(18, 5)), LSB(NTADR_A(18, 5)), 'B',
                              NT_UPD_EOF}; // data must end in EOF

  ppu_off(); // screen off

  pal_bg(palette); //	load the palette

  ppu_on_all(); // turn on screen

  set_vram_update(text); // set a pointer to the data to transfer during nmi

  ppu_wait_nmi(); // waits until the next nmi is completed, also sets a VRAM
                  // update flag the text will be auto pushed to the PPU during
                  // nmi

  set_vram_update(two_letters); // set a pointer to the data

  ppu_wait_nmi(); // the two_letters will be auto pushed to the PPU in the next
                  // nmi

  set_vram_update(
      NULL); // just turns off the VRAM update system so that it
             // isn't wasting time writing the same data to the PPU every frame

  while (1) {
    // infinite loop
    // game code can go here later.
  }
}
