/*	simple Hello World, for llvm-mos, for NES
 *  writing to the screen with rendering disabled
 *	using neslib
 *	Doug Fraker 2018
 */

#include <neslib.h>

#define BLACK 0x0f
#define DK_GY 0x00
#define LT_GY 0x10
#define WHITE 0x30

int main(void) {
  static const char text[] = "Hello World!"; // zero terminated c string
  static const char palette[] = {BLACK, DK_GY, LT_GY, WHITE, [15] = 0};

  ppu_off(); // screen off

  pal_bg(palette); //	load the BG palette

  // set a starting point on the screen
  // vram_adr(NTADR_A(x,y));
  vram_adr(NTADR_A(10, 14)); // screen is 32 x 30 tiles

  for (char i = 0; text[i]; ++i)
    vram_put(text[i]); // this pushes 1 char to the screen

  // vram_adr and vram_put only work with screen off
  // NOTE, you could replace everything between i = 0; and here with...
  // vram_write(text,sizeof(text));
  // does the same thing

  ppu_on_all(); //	turn on screen

  while (1) {
    // infinite loop
    // game code can go here later.
  }
}
