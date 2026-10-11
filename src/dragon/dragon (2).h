#ifndef DRAGON_H
#define DRAGON_H

#include <stdint.h>
#include <stddef.h>

// Cấu trúc lưu trữ trạng thái của Dragon Cipher
typedef struct {
    uint32_t B[32]; // NLFSR 1024-bit
    uint32_t ML;    // Memory Left 32-bit
    uint32_t MR;    // Memory Right 32-bit
} DragonState;

// Hàm khởi tạo trạng thái với Khóa (Key 256-bit) và Vector khởi tạo (IV 256-bit)
void dragon_init(DragonState* ctx, const uint8_t key[32], const uint8_t iv[32]);

// Hàm mã hóa/giải mã dữ liệu
// Vì là stream cipher, hàm này dùng chung cho cả encrypt và decrypt
void dragon_encrypt(DragonState* ctx, const uint8_t* input, uint8_t* output, size_t length);

#endif // DRAGON_H