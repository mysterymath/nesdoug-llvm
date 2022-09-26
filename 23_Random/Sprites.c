
#include <neslib.h>

// clang-format off
const unsigned char metasprite[] = {
  0, 0, 0x01, 0,
  0, 8, 0x11, 0,
  8, 0, 0x01, 0 | OAM_FLIP_H,
  8, 8, 0x11, 0 | OAM_FLIP_H,
  128
};
// clang-format on

// clang-format off
const unsigned char metasprite2[] = {
  8,  0,  0x03, 0,
  0,  8,  0x12, 0,
  8,  8,  0x13, 0,
  16, 8,  0x12, 0 | OAM_FLIP_H,
  0,  16, 0x22, 0,
  8,  16, 0x23, 0,
  16, 16, 0x22, 0 | OAM_FLIP_H,
  128
};
// clang-format on
