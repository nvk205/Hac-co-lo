#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "dragon.h"
#include "dragon_init.h"
#include "dragon_f.h"
#include "dragon_cipher.h"
#include "dragon_sbox.h"

#define CALL_INIT_KEY_IV(ctx, key, iv, bits) \ dragon_setup_key_iv((ctx), (key), (iv), (bits))

#define CALL_F_TRACE(a, b, c, d, e, f, trace_ptr) \ dragon_F_with_trace((a), (b), (c), (d), (e), (f), (trace_ptr))

#define CALL_XCRYPT_BUF(key, iv, bits, in, out, len) \ dragon_xcrypt_buffer((key), (iv), (bits), (in), (out), (len))

#define CALL_XCRYPT_FILE(in_path, out_path, key, iv, bits) \ dragon_xcrypt_file((in_path), (out_path), (key), (iv), (bits))

static int hex_char_to_val(char c){
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;
}

// Chuyển chuỗi Hex sang mảng byte. Trả về số byte đã chuyển đổi 
static size_t hex_to_bytes(const char *hex_str, uint8_t *out_bytes, size_t max_bytes) {
    size_t len = strlen(hex_str);
    size_t count = 0;
    for (size_t i = 0; i + 1 < len && count < max_bytes; i += 2) {
        int hi = hex_char_to_val(hex_str[i]);
        int lo = hex_char_to_val(hex_str[i + 1]);
        if (hi < 0 || lo < 0) break;
        out_bytes[count++] = (uint8_t)((hi << 4) | lo);
    }
    return count;
}

// In mảng byte dưới dạng chuỗi Hex viết hoa
static void print_bytes_hex(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02X", data[i]);
    }
}

// In mảng 32 từ 32-bit dưới dạng mảng JSON 
static void print_json_u32_array(const uint32_t *arr, size_t n) {
    printf("[");
    for (size_t i = 0; i < n; i++) {
        printf("\"0x%08X\"%s", arr[i], (i + 1 < n) ? ", " : "");
    }
    printf("]");
}

int main(int argc, char *argv[]) {
    //Cài đặt menu dòng lệnh hoặc xử lý tham số argv[]

    return 0;
}