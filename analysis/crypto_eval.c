#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

#include "src/dragon/dragon.h"

#define STREAM_SIZE (1024 * 1024)   // 1 MB keystream

/* Số bit cần so sánh trong thí nghiệm avalanche */
#define AVALANCHE_BITS 1000
#define AVALANCHE_BYTES (AVALANCHE_BITS / 8)

/* Hàm đến số bit 1 trong 1 từ mã 32 bit */
static unsigned int dem_bit_1(uint32_t x) {
    unsigned int count = 0;

    while (x != 0) {
        x &= x - 1;   // Xóa bit 1 thấp nhất
        ++count;
    }

    return count;
}

/* Hàm tính khoảng cách Hamming giữa 2 trạng thái 1024 bit */
static unsigned int dem_bit_khac_nha(const uint32_t state_a[32], const uint32_t state_b[32]) {
    unsigned int so_bit_khac_nhau = 0;

    for (unsigned int i = 0; i < 32; ++i) {
        so_bit_khac_nhau += dem_bit_1(state_a[i] ^ state_b[i]);
    }

    return so_bit_khac_nhau;
}


