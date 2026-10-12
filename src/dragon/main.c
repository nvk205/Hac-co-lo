#include <stdio.h>
#include <inttypes.h>
#include "dragon.h"

int main(void)
{
    u64 key[2] = {
        0x0000111122223333ULL,
        0x4444555566667777ULL
    };
    u64 iv[2] = {
        0x0000111122223333ULL,
        0x4444555566667777ULL
    };

    element W[8];
    u_int B[32];
    u64 z;
    int j, k;

    init_key(key, iv, W, 1);

    /* Chuy?n W[8] thành 32 t? 32 bit trong B[32]. */
    for (j = 0, k = 0; k < 8; k++, j += 4) {
        B[j] = (u_int)(W[k].lb >> 32);
        B[j + 1] = (u_int)W[k].lb;
        B[j + 2] = (u_int)(W[k].rb >> 32);
        B[j + 3] = (u_int)W[k].rb;
    }

    /* Sinh và in 16 giá tr? ??u ra 64 bit. */
    for (int x = 0; x < 16; x++) {
        gen_key(B, &z);
        printf("%016" PRIx64 "\n", z);
    }

    return 0;
}
