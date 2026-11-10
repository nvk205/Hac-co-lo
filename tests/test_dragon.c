#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "../src/dragon/dragon.h"
#include "../src/dragon/dragon_sbox.h"
#include "../src/dragon/dragon_f.h"
#include "../src/dragon/dragon_init.h"
#include "../src/dragon/dragon_cipher.h"

/*
 1. test_sbox_and_G(): Kiểm tra hàm G, H.
 2. test_f_function(): Kiểm tra 1 vòng hàm dragon_F.
 3. test_encrypt_decrypt_roundtrip(): Kiểm tra mã hóa 1 chuỗi tiếng Việt bất kỳ
    rồi giải mã lại xem có khớp 100% với bản rõ ban đầu không.
 4. test_estream_vectors(): Đọc dữ liệu từ file tests/vectors.txt và đối chiếu
    Keystream sinh ra cho cả Key 128-bit và Key 256-bit.
*/

int test_sbox_and_G(void) {
    // Viết test kiểm tra hàm G, H
    return 1;
}

int test_f_function(void) {
    // Viết test kiểm tra hàm F
    return 1;
}

int test_encrypt_decrypt_roundtrip(void) {
    // Mã hóa thử 1 câu dài 37 byte (lẻ byte) rồi giải mã kiểm tra
    return 1;
}

int test_estream_vectors(const char *vector_filepath) {
    // Đối chiếu với bộ vector chuẩn trong tests/vectors.txt
    return 1;
}

int main(void) {
    //Gọi 4 hàm test 
    return 0;
}