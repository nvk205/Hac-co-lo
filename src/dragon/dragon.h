#ifndef DRAGON_H
#define DRAGON_H

#include <stdint.h>
#include <stdio.h>
#include "dragon_sbox.h"

typedef struct {
    uint32_t B[64]; // 64 phần tử 32 bit
    uint32_t M_L;
    uint32_t M_H;
} Dragon_Ctx;

void dragon_init_ctx(Dragon_Ctx *ctx);
void dragon_shift_nlfsr(Dragon_Ctx *ctx, uint32_t new_B30, uint32_t new_B31);
void dragon_generate_keystream(Dragon_Ctx *ctx, uint32_t *ks_word1, uint32_t *ks_word2);
void dragon_print_state(const Dragon_Ctx *ctx);

#endif