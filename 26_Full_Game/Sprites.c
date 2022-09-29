// a 16x16 pixel metasprite

const unsigned char RoundSprL[] = {
    // clang-format off
    0xff, 0xfc, 0x02, 0,
    7,    0xfc, 0x03, 0,
    0xff, 4,    0x12, 0,
    7,    4,    0x13, 0,
    128
    // clang-format on
};

const unsigned char RoundSprR[] = {
    // clang-format off
    0xff, 0xfc, 0x00, 0,
    7,    0xfc, 0x01, 0,
    0xff, 4,    0x10, 0,
    7,    4,    0x11, 0,
    128
    // clang-format on
};

const unsigned char CoinSpr[] = {
    // clang-format off
    0xff, 0xff, 0x20, 1,
    0xff, 7,    0x30, 1,
    128
    // clang-format on
};

const unsigned char BigCoinSpr[] = {
    // clang-format off
    0xff, 0xff, 0x21, 1,
    0xff, 7,    0x31, 1,
    7,    0xff, 0x22, 1,
    7,    7,    0x32, 1,
    128
    // clang-format on
};

const unsigned char CoinHud[] = {
    // clang-format off
    0, 0, 0x23, 1,
    8, 0, 0x24, 1,
    0, 8, 0x33, 1,
    8, 8, 0x34, 1,
    128
    // clang-format on
};

const unsigned char EnemyChaseSpr[] = {
    // clang-format off
    0xff, 0xff, 0x04, 2,
    7,    0xff, 0x05, 2,
    0xff, 7,    0x14, 2,
    7,    7,    0x15, 2,
    128
    // clang-format on
};

const unsigned char EnemyBounceSpr[] = {
    // clang-format off
    0xff, 0xff, 0x06, 3,
    7,    0xff, 0x07, 3,
    0xff, 7,    0x16, 3,
    7,    7,    0x17, 3,
    128
    // clang-format on
};

const unsigned char EnemyBounceSpr2[] = {
    // clang-format off
    0xff, 0xff, 0x04, 3,
    7,    0xff, 0x05, 3,
    0xff, 7,    0x14, 3,
    7,    7,    0x15, 3,
    128
    // clang-format on
};
