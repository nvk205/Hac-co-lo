#ifndef DRAGON_CIPHER_H
#define DRAGON_CIPHER_H

#include <stdint.h>
#include <stddef.h>

// Mã hóa hoặc giải mã một vùng nhớ có độ dài byte bất kỳ
int dragon_xcrypt_buffer(const uint8_t *key, const uint8_t *iv, uint32_t key_bits,
                         const uint8_t *input, uint8_t *output, size_t length);

// Mã hóa hoặc giải mã trực tiếp một file bất kỳ
int dragon_xcrypt_file(const char *in_filepath, const char *out_filepath,
                       const uint8_t *key, const uint8_t *iv, uint32_t key_bits);

#endif

