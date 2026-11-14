#include "dragon.h"
#include "dragon_f.h" // kết nối hàm của Tâm
void dragon_init_ctx(Dragon_Ctx *ctx) {
    if (!ctx) return;
    for (int i = 0; i < 32; i++) {
        ctx->B[i] = 0;
    }
    ctx->M_L = 0;
    ctx->M_H = 0;
}

void dragon_shift_nlfsr(Dragon_Ctx *ctx, uint32_t new_B0, uint32_t new_B1) {
    for (int i = 31; i >= 2; i--) {
        ctx->B[i] = ctx->B[i - 2];
    }
    ctx->B[0] = new_B0;
    ctx->B[1] = new_B1;
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
// Gọi hàm cập nhật trạng thái F của Thành viên 2 (cập nhật trực tiếp a..f)
    dragon_F(&a, &b, &c, &d, &e, &f);
    //Dịch thanh ghi B đi 2 ô và nạp 2 từ phản hồi (a', d') vào đầu mảng
    dragon_shift_nlfsr(ctx, a, d);
    // Xuất 64-bit khóa dòng (z0 = a', z1 = e')
    *ks_word1 = a;
    *ks_word2 = e;
}

void dragon_print_state(const Dragon_Ctx *ctx) {
    if (!ctx) return;
    printf("=== TRANG THAI NLFSR (M = %08X%08X) ===\n", ctx->M_H, ctx->M_L);
    for (int i = 0; i < 32; i++) {
        printf("B[%02d]: 0x%08X%s", i, ctx->B[i], (i % 4 == 3) ? "\n" : "  ");
    }
    printf("\n");
}