#ifndef DRAGON_F_H
#define DRAGON_F_H

#include <stdint.h>
#include "dragon_sbox.h"

/* 
 1. Cài đặt hàm F nhận vào 6 từ 32-bit (a, b, c, d, e, f) qua 3 tầng:
    - Tầng 1 (Pre-mixing): XOR và cộng modulo 2^32.
    - Tầng 2 (S-box layer): Gọi G_box / H_box của Huấn
    - Tầng 3 (Post-mixing): Cộng modulo 2^32 và XOR.
 2. Cài đặt hàm F_trace lưu lại giá trị sau mỗi tầng để gửi lên Giao diện (GUI).
*/

typedef struct {
    uint32_t in[6];
    uint32_t pre_mix[6];
    uint32_t sbox_layer[6];
    uint32_t out[6];
} Dragon_F_Trace;

void dragon_F(uint32_t *a, uint32_t *b, uint32_t *c,
              uint32_t *d, uint32_t *e, uint32_t *f);

void dragon_F_trace(uint32_t *a, uint32_t *b, uint32_t *c,
                    uint32_t *d, uint32_t *e, uint32_t *f,
                    Dragon_F_Trace *trace);

#endif