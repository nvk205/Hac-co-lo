#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "dragon_f.h"

/* Bốn hàng lần lượt là đầu vào, sau trộn đầu, sau tầng phi tuyến và đầu ra. */
static const uint32_t mau_co_ban[4][6] = {
    {1, 2, 3, 4, 5, 6},
    {4, 3, 6, 7, 12, 3},
    {0x7C2D3A52, 0x4ABFBC25, 0x722694ED,
     0xB718E7D4, 0x80E15A11, 0xE801F7B0},
    {0x2605B6CF, 0xCBA11636, 0xB98782DB,
     0x33462226, 0xB3A77837, 0x5A288C9D}
};

