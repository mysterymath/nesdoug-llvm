#ifndef ROOMS_H
#define ROOMS_H

// data is exactly 240 bytes, 16 * 15
// doubles as the collision map data
extern const unsigned char *const Rooms[];

#define MAX_ROOMS (5 - 1)
#define MAX_SCROLL (MAX_ROOMS * 0x100 - 1)

#endif // ROOMS_H
