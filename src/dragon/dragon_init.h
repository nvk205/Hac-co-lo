#ifndef DRAGON_INIT_H
#define DRAGON_INIT_H

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "dragon.h"


typedef struct {
    unsigned long long lb; //64 bit cao.
    unsigned long long rb; //64 bit thấp.
} element; //DELETE

/*
1. Update init_key(...) để nạp Key và IV trực tiếp vào 32 ô của mảng ctx->B.
2. Bỏ cấu trúc 'element' cũ (lb, rb). 
   Sử dụng struct Dragon_Ctx (gồm mảng uint32_t B[32], M_L, M_H) do Chinh định nghĩa.
*/

void init_key(Dragon_Ctx *ctx, const uint8_t *key, const uint8_t *iv, uint32_t key_size);

#endif