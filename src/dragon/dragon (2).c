#include "dragon.h"
#include <string.h>

// Bảng S-BOX nội bộ
static uint32_t S1[256];
static uint32_t S2[256];

// Hàm khởi tạo S-Box theo chuẩn nội bộ (chỉ gọi ẩn bên trong)
static void init_sboxes() {
    uint32_t lcg_state = 0x12345678;
    for (int i = 0; i < 256; i++) {
        lcg_state = (lcg_state * 1103515245) + 12345;
        S1[i] = lcg_state;
        lcg_state = (lcg_state * 1103515245) + 12345;
        S2[i] = lcg_state;
    }
}

// Trích xuất byte
#define BYTE(x, n) (((x) >> (8 * (3 - (n)))) & 0xFF)

// Các hàm G và H (Feistel-like Substitution)
static inline uint32_t G1(uint32_t x) { return S1[BYTE(x, 0)] ^ S1[BYTE(x, 1)] ^ S1[BYTE(x, 2)] ^ S2[BYTE(x, 3)]; }
static inline uint32_t G2(uint32_t x) { return S1[BYTE(x, 0)] ^ S1[BYTE(x, 1)] ^ S2[BYTE(x, 2)] ^ S1[BYTE(x, 3)]; }
static inline uint32_t G3(uint32_t x) { return S1[BYTE(x, 0)] ^ S2[BYTE(x, 1)] ^ S1[BYTE(x, 2)] ^ S1[BYTE(x, 3)]; }
static inline uint32_t H1(uint32_t x) { return S2[BYTE(x, 0)] ^ S2[BYTE(x, 1)] ^ S2[BYTE(x, 2)] ^ S1[BYTE(x, 3)]; }
static inline uint32_t H2(uint32_t x) { return S2[BYTE(x, 0)] ^ S2[BYTE(x, 1)] ^ S1[BYTE(x, 2)] ^ S2[BYTE(x, 3)]; }
static inline uint32_t H3(uint32_t x) { return S2[BYTE(x, 0)] ^ S1[BYTE(x, 1)] ^ S2[BYTE(x, 2)] ^ S2[BYTE(x, 3)]; }

// Hàm Cập nhật phi tuyến lõi (F-function)
static void dragon_F(uint32_t* a, uint32_t* b, uint32_t* c, uint32_t* d, uint32_t* e, uint32_t* f) {
    *b ^= *a; *d ^= *c; *f ^= *e;
    *c += *b; *e += *d; *a += *f;

    *d ^= G1(*a); *f ^= G2(*c); *b ^= G3(*e);
    *a ^= H1(*b); *c ^= H2(*d); *e ^= H3(*f);

    *d += *a; *f += *c; *b += *e;
    *c ^= *b; *e ^= *d; *a ^= *f;
}

// Hàm khởi tạo thuật toán
void dragon_init(DragonState* ctx, const uint8_t key[32], const uint8_t iv[32]) {
    init_sboxes();

    for (int i = 0; i < 8; i++) {
        uint32_t kw = (key[i * 4] << 24) | (key[i * 4 + 1] << 16) | (key[i * 4 + 2] << 8) | key[i * 4 + 3];
        uint32_t iw = (iv[i * 4] << 24) | (iv[i * 4 + 1] << 16) | (iv[i * 4 + 2] << 8) | iv[i * 4 + 3];

        ctx->B[i] = kw;
        ctx->B[i + 8] = iw;
        ctx->B[i + 16] = kw ^ iw;
        ctx->B[i + 24] = ~(kw ^ iw);
    }
    ctx->ML = 0x44524147;
    ctx->MR = 0x4F4E2020;

    for (int r = 0; r < 16; r++) {
        uint32_t a = ctx->B[0], b = ctx->B[9], c = ctx->B[16];
        uint32_t d = ctx->B[19], e = ctx->B[30] ^ ctx->ML, f = ctx->B[31] ^ ctx->MR;

        dragon_F(&a, &b, &c, &d, &e, &f);
        ctx->ML = e; ctx->MR = f;

        memmove(&ctx->B[0], &ctx->B[2], 30 * sizeof(uint32_t));
        ctx->B[30] = a; ctx->B[31] = d;
    }
}

// Hàm mã hóa/giải mã
void dragon_encrypt(DragonState* ctx, const uint8_t* input, uint8_t* output, size_t length) {
    uint8_t keystream[8];

    for (size_t i = 0; i < length; i++) {
        if (i % 8 == 0) {
            uint32_t a = ctx->B[0], b = ctx->B[9], c = ctx->B[16];
            uint32_t d = ctx->B[19], e = ctx->B[30] ^ ctx->ML, f = ctx->B[31] ^ ctx->MR;

            dragon_F(&a, &b, &c, &d, &e, &f);

            keystream[0] = (b >> 24) & 0xFF; keystream[1] = (b >> 16) & 0xFF;
            keystream[2] = (b >> 8) & 0xFF;  keystream[3] = b & 0xFF;
            keystream[4] = (c >> 24) & 0xFF; keystream[5] = (c >> 16) & 0xFF;
            keystream[6] = (c >> 8) & 0xFF;  keystream[7] = c & 0xFF;

            ctx->ML = e; ctx->MR = f;
            memmove(&ctx->B[0], &ctx->B[2], 30 * sizeof(uint32_t));
            ctx->B[30] = a; ctx->B[31] = d;
        }
        output[i] = input[i] ^ keystream[i % 8];
    }
}