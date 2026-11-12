#include "dragon_cipher.h"
#include "dragon.h"
#include "dragon_init.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
static void bytes_to_words(const uint8_t *bytes, uint32_t *words, size_t num_words) {
    for (size_t i = 0; i < num_words; i++) {     
        words[i] = ((uint32_t)bytes[4 * i]     << 24) |
                   ((uint32_t)bytes[4 * i + 1] << 16) |
                   ((uint32_t)bytes[4 * i + 2] << 8)  |
                   ((uint32_t)bytes[4 * i + 3]);
    }
}
// Khởi tạo trạng thái Dragon_Ctx từ key và iv 
static int setup_cipher_state(Dragon_Ctx *W, const uint8_t *key, const uint8_t *iv, uint32_t key_bits) {
    //Định nghĩa độ dài khóa chuẩn là 128-bit hoặc 256-bit
    if (key_bits != 128 && key_bits != 256) {
        return -2;
    }
    Dragon_Ctx key_ctx, iv_ctx;
    dragon_init_ctx(&key_ctx);
    dragon_init_ctx(&iv_ctx);
    dragon_init_ctx(W);
    //128-bit = 4 từ và 256-bit = 8 từ
    size_t num_words = (key_bits == 128) ? 4 : 8;
    // Trong dragon_init.c: n=1 là 128-bit, n=0 là 256-bit
    int mode = (key_bits == 128) ? 1 : 0; 

    bytes_to_words(key, key_ctx.B, num_words);
    bytes_to_words(iv, iv_ctx.B, num_words);
    // Gọi hàm dragon_init.c để trộn key và iv qua 16 vòng
    init_key(&key_ctx, &iv_ctx, W, mode);
    return 0;
}

static void get_next_keystream_block(Dragon_Ctx *ctx, uint8_t block[8]) {
    uint32_t w1 = 0, w2 = 0;
    
    dragon_generate_keystream(ctx, &w1, &w2);
    // Tách từ 32-bit thứ nhất (w1) thành 4 byte đầu của mảng
    // Dịch phải và & 0xFF để xóa bit cao
    block[0] = (uint8_t)((w1 >> 24) & 0xFF); 
    block[1] = (uint8_t)((w1 >> 16) & 0xFF); 
    block[2] = (uint8_t)((w1 >> 8)  & 0xFF); 
    block[3] = (uint8_t)(w1 & 0xFF);         

    // Tách từ 32-bit thứ hai (w2) thành 4 byte tiếp của mảng
    block[4] = (uint8_t)((w2 >> 24) & 0xFF);
    block[5] = (uint8_t)((w2 >> 16) & 0xFF);
    block[6] = (uint8_t)((w2 >> 8)  & 0xFF);
    block[7] = (uint8_t)(w2 & 0xFF);
}