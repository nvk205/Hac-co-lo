#include "dragon.h"

void dragon_init_ctx(Dragon_Ctx *ctx) {
    if (!ctx) return;
    for (int i = 0; i < 32; i++) {
        ctx->B[i] = 0;
    }
    ctx->M_L = 0;
    ctx->M_H = 0;
}

void dragon_shift_nlfsr(Dragon_Ctx *ctx, uint32_t new_B30, uint32_t new_B31) {
    for (int i = 0; i < 30; i++) {
        ctx->B[i] = ctx->B[i + 2];
    }
    ctx->B[30] = new_B30;
    ctx->B[31] = new_B31;
}

void dragon_generate_keystream(Dragon_Ctx *ctx, uint32_t *ks_word1, uint32_t *ks_word2) {
    if (!ctx || !ks_word1 || !ks_word2) return;

    uint32_t a = ctx->B[0];
    uint32_t b = ctx->B[9];
    uint32_t c = ctx->B[16];
    uint32_t d = ctx->B[19];
    uint32_t e = ctx->B[30] ^ ctx->M_H;
    uint32_t f = ctx->B[31] ^ ctx->M_L;

    ctx->M_L++;
    if (ctx->M_L == 0) {
        ctx->M_H++;
    }

    // Tạm gán để biên dịch chạy thử trước khi ghép với Tâm
    *ks_word1 = e;
    *ks_word2 = f;
    dragon_shift_nlfsr(ctx, a ^ b, c ^ d);
}

void dragon_print_state(const Dragon_Ctx *ctx) {
    if (!ctx) return;
    printf("=== TRANG THAI NLFSR (M = %08X%08X) ===\n", ctx->M_H, ctx->M_L);
    for (int i = 0; i < 32; i++) {
        printf("B[%02d]: 0x%08X%s", i, ctx->B[i], (i % 4 == 3) ? "\n" : "  ");
    }
    printf("\n");
}