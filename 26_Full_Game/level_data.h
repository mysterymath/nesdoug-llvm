#ifndef LEVEL_DATA_H
#define LEVEL_DATA_H

#include "BG/Levels.h"

enum { COIN_REG, COIN_END };

// NOTE MAX_COINS = 12

// y, room, x, type
// y = TURN_OFF end of list
extern const unsigned char *const Coins_list[];

enum { ENEMY_CHASE, ENEMY_BOUNCE };

extern const unsigned char *const Enemy_list[];

extern const unsigned char metatiles1[];

#define COL_DOWN 0x80
#define COL_ALL 0x40

extern const unsigned char is_solid[];

extern const unsigned char *const Levels_list[];

extern const unsigned char Level_offsets[];

#define MAX_ROOMS (8 - 1)
#define MAX_SCROLL (MAX_ROOMS * 0x100 - 1)
#define TURN_OFF 0xff

#endif // LEVEL_DATA_H
