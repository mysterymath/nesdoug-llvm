#include "Rooms.h"

extern const unsigned char Room1[];
extern const unsigned char Room2[];
extern const unsigned char Room3[];
extern const unsigned char Room4[];
extern const unsigned char Room5[];

const unsigned char *const Rooms[] = {Room1, Room2, Room3, Room4, Room5};

//y, room, x
//y = TURN_OFF end of list
// clang-format off
const char level_1_coins[] = {
  0x61, 0, 0x65,
  0x81, 1, 0x85,
  0x41, 2, 0x95,
  0x61, 3, 0x75,
  0x41, 4, 0x85,
  TURN_OFF
};
// clang-format off

//y, room, x
//y = TURN_OFF end of list
// clang-format on
const char level_1_enemies[] = {
  0xc2, 0, 0xb0,
  0xc2, 1, 0x80,
  0xc2, 2, 0xc0,
  0xc2, 3, 0xf0,
  TURN_OFF
};
// clang-format off
