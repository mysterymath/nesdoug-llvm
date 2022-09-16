#define SPEED 0x180

#pragma bss - name(push, "ZEROPAGE")

// GLOBAL VARIABLES
unsigned char pad1;
unsigned char pad1_new;
unsigned char collision;
unsigned char collision_L;
unsigned char collision_R;
unsigned char collision_U;
unsigned char collision_D;
unsigned char coordinates;
unsigned char temp1;
unsigned char temp2;
unsigned char temp3;
unsigned char temp4;
unsigned int temp5;
unsigned int temp6;
unsigned char eject_L;   // from the left
unsigned char eject_R;   // remember these from the collision sub routine
unsigned char eject_D;   // from below
unsigned char eject_U;   // from up
unsigned char direction; // facing left or right?
#define LEFT 0
#define RIGHT 1

int address;
unsigned char x; // room loader code
unsigned char y;
unsigned char index;
unsigned char map;
unsigned int scroll_x;
unsigned int scroll_y;
signed int hero_velocity_x; // signed, low byte is sub-pixel
signed int hero_velocity_y;
unsigned int hero_x;
unsigned int hero_y;
unsigned char L_R_switch;
unsigned char old_x;
unsigned char old_y;
unsigned char stick;

#pragma bss - name(push, "BSS")

unsigned char c_map[240];
unsigned char c_map2[240]; // not used in this example

struct Base {
  unsigned char x;
  unsigned char y;
  unsigned char width;
  unsigned char height;
};

struct Base Generic;

struct BoxGuy {
  unsigned char x;
  unsigned char y;
};

struct BoxGuy BoxGuy1 = {0x40, 0x30};
// the width and height should be 1 less than the dimensions (16x16)
// ...I shrunk it a bit 14x14 hitbox
// note, I'm using the top left as the 0,0 on x,y

#define HERO_WIDTH 13
#define HERO_HEIGHT 13

// 5 bytes per metatile definition, tile TL, TR, BL, BR, palette 0-3
// T means top, B means bottom, L left,R right
// 51 maximum # of metatiles = 255 bytes

#include "BG/Room1.c"
// data is exactly 240 bytes, 16 * 15
// doubles as the collision map data

// PROTOTYPES
void load_room(void);
void draw_sprites(void);
void movement(void);
void bg_collision(void);
void bg_collision_sub(void);
