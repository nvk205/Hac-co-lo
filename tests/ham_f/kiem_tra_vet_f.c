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

/* Ở tầng trộn đầu, a bằng 0xFFFFFFFF cộng với 1 nên quay về 0. */
static const uint32_t mau_tran[4][6] = {
    {0xFFFFFFFF, 0, 0, 0, 0, 1},
    {0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 1},
    {0x3BDB29EF, 0x6F8FF210, 0x5BF81117,
     0x90700DEF, 0x71F299F3, 0x50C4AB23},
    {0x976795D5, 0xE1828C03, 0xBA7A9D14,
     0xCC4B37DE, 0xBDB9AE2D, 0xACBCBC3A}
};

static int kiem_tra(const char *ten, const uint32_t mau[4][6])
{
    uint32_t x[6], y[6];


    /* Hai hàm đều sửa trực tiếp sáu biến nên mỗi hàm cần một bản sao đầu vào. */
    memcpy(x, mau[0], sizeof x);
    memcpy(y, mau[0], sizeof y);


    dragon_F(&y[0], &y[1], &y[2], &y[3], &y[4], &y[5]);

    const uint32_t *cac_tang[] = {vet.in, vet.pre_mix, vet.sbox_layer, vet.out};
    const char *ten_tang[] = {"đầu vào", "trộn đầu", "phi tuyến", "trộn cuối"};

    for (int tang = 0; tang < 4; tang++) {
        for (int i = 0; i < 6; i++) {
            if (cac_tang[tang][i] != mau[tang][i]) {
                printf("SAI - %s, tầng %s, biến %c\n", ten, ten_tang[tang], "abcdef"[i]);
                return 1;
            }
        }
    }

    /* Kiểm tra cả sáu biến mà bên gọi nhận lại sau khi hàm chạy xong. */
    if (memcmp(x, mau[3], sizeof x) != 0) {
        printf("SAI - %s, đầu ra của dragon_F_trace\n", ten);
        return 1;
    }
    if (memcmp(y, mau[3], sizeof y) != 0) {
        printf("SAI - %s, đầu ra của dragon_F\n", ten);
        return 1;
    }

    printf("ĐẠT - %s\n", ten);
    return 0;
}

int main(void)
{
    return kiem_tra("Bộ cơ bản", mau_co_ban);
}