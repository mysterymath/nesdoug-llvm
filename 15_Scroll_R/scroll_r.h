#define SPEED 0x180
#define MAX_RIGHT 0xb000



#pragma bss-name(push, "ZEROPAGE")

// GLOBAL VARIABLES
unsigned char pad1;
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
unsigned char eject_L; // from the left
unsigned char eject_R; // remember these from the collision sub routine
unsigned char eject_D; // from below
unsigned char eject_U; // from up
unsigned char direction; //facing left or right?
#define LEFT 0
#define RIGHT 1

int address;
unsigned char x; // room loader code
unsigned char y;
unsigned char nt;
unsigned char index;
unsigned char room;
unsigned char map;
unsigned int scroll_x;
unsigned int scroll_y;
unsigned char scroll_count; 
unsigned int pseudo_scroll_x; 
//unsigned int pseudo_scroll_y;
unsigned char L_R_switch;
unsigned int old_x;
unsigned int old_y;





#pragma bss-name(push, "BSS")

unsigned char c_map[240];
unsigned char c_map2[240];

struct Base {
	unsigned char x;
	unsigned char y;
	unsigned char width;
	unsigned char height;
};

struct Base Generic; 

struct Hero {
	unsigned int x; // low byte is sub-pixel
	unsigned int y;
	signed int vel_x; // speed, signed, low byte is sub-pixel
	signed int vel_y;
};

struct Hero BoxGuy1 = {0x4000,0xc400}; // starting position
// the width and height should be 1 less than the dimensions (14x14)
// note, I'm using the top left as the 0,0 on x,y

#define HERO_WIDTH 13
#define HERO_HEIGHT 13







const unsigned char palette_bg[]={
0x0f, 0x00, 0x10, 0x30, // black, gray, lt gray, white
0x0f, 0x07, 0x17, 0x27, // oranges
0x0f, 0x02, 0x12, 0x22, // blues
0x0f, 0x09, 0x19, 0x29, // greens
}; 

const unsigned char palette_sp[]={
0x0f, 0x07, 0x28, 0x38, // dk brown, yellow, white yellow
0,0,0,0,
0,0,0,0,
0,0,0,0
}; 


// 5 bytes per metatile definition, tile TL, TR, BL, BR, palette 0-3
// T means top, B means bottom, L left,R right
// 51 maximum # of metatiles = 255 bytes

const unsigned char metatiles1[]={
	2, 2, 2, 2,  3,
	4, 4, 4, 4,  1,
	9, 9, 9, 9,  2,
	5, 6, 8, 7,  1,
	5, 6, 8, 7,  0
};



#include "BG/Room1.c"
#include "BG/Room2.c"
#include "BG/Room3.c"
#include "BG/Room4.c"
#include "BG/Room5.c"

#define MAX_ROOMS (5-1)
#define MAX_SCROLL (MAX_ROOMS*0x100)-1
// data is exactly 240 bytes, 16 * 15
// doubles as the collision map data


const unsigned char * const Rooms[]= {
	Room1, Room2, Room3, Room4, Room5
};


// PROTOTYPES
void load_room(void);
void draw_sprites(void);
void movement(void);	
void bg_collision(void);
void draw_screen_R(void);
void new_cmap(void);
void bg_collision_sub(void);

