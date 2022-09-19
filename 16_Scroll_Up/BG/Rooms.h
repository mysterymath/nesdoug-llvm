#ifndef BG_ROOMS_H
#define BG_ROOMS_H

#define MAX_ROOMS (5 - 1)
#define MAX_SCROLL ((MAX_ROOMS * 0x100) - 0x11)

#define MIN_SCROLL 2
// data is exactly 240 bytes, 16 * 15
// doubles as the collision map data

extern const unsigned char *const Rooms[];

#endif // BG_ROOMS_H
