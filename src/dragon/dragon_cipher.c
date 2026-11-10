#include "dragon_cipher.h"
#include "dragon_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void dragon_encrypt_buffer(const uint8_t *input, size_t input_len, uint8_t *output, const uint8_t *key, const uint8_t *iv) {

    // Khai báo mảng trạng thái 16 phần tử
    uint32_t state[16]; 
    dragon_init(state, key, iv);
    size_t offset = 0;

    // Khai báo mảng chứa khối khóa dòng 8 byte
    uint8_t block[8];

    // Vòng lặp xử lý các khối đủ 8 byte
    while (offset + 8 <= input_len) {
    
        dragon_get_next_block(state, block);
        
        for (int i = 0; i < 8; i++) {
            output[offset + i] = input[offset + i] ^ block[i];
        }
    
    if (offset < input_len) {

        dragon_get_next_block(state, block);
        size_t rem = input_len - offset;
    
        for (size_t i = 0; i < rem; i++) {
            output[offset + i] = input[offset + i] ^ block[i];
        }
    }
}