#ifndef DRAGON_CIPHER_H
#define DRAGON_CIPHER_H

#include <stdint.h>
#include <stddef.h>

// Khởi tạo trạng thái ban đầu
void dragon_init(uint32_t *state, const uint8_t *key, const uint8_t *iv);

void dragon_get_next_block(uint32_t *state, uint8_t block[8]);

// Mã hóa vùng nhớ buffer với độ dài bất kỳ
void dragon_encrypt_buffer(const uint8_t *input, size_t input_len,  uint8_t *output, const uint8_t *key, const uint8_t *iv);

#endif 