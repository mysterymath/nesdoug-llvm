#include <neslib.h>

// a 16x16 pixel metasprite

const unsigned char StarDark[] = {
    // clang-format off
    0, 0, 0x01, 0,
    8, 0, 0x01, 0 | OAM_FLIP_H,
    0, 8, 0x11, 0,
    8, 8, 0x11, 0 | OAM_FLIP_H,
    128
    // clang-format on
};

const unsigned char StarLight[] = {
    // clang-format off
    0, 0, 0x02, 0,
    8, 0, 0x02, 0 | OAM_FLIP_H,
    0, 8, 0x12, 0,
    8, 8, 0x12, 0 | OAM_FLIP_H,
    128
    // clang-format on
};

const unsigned char WhiteBox[] = {
    // clang-format off
    0xfc, 0xfc, 0x00, 0,
    4,    0xfc, 0x00, 0,
    12,   0xfc, 0x00, 0,
    0xfc, 4,    0x00, 0,
    4,    4,    0x00, 0,
    12,   4,    0x00, 0,
    0xfc, 12,   0x00, 0,
    4,    12,   0x00, 0,
    12,   12,   0x00, 0,
    128
    // clang-format on
};
