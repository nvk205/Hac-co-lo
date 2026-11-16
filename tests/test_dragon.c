#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include "test_dragon.h"
#include "../src/dragon/dragon.h"
#include "../src/dragon/dragon_sbox.h"
#include "../src/dragon/dragon_f.h"
#include "../src/dragon/dragon_init.h"
#include "../src/dragon/dragon_cipher.h"

int test_Hbox_and_Gbox(const char *filepath) {
    printf("TEST G_box & H_box (file: %s)\n", filepath);

    // Mở file chứa các test case
    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("Khong the mo file test: %s\n", filepath);
        return 0;
    }

    char line[256];
    int line_num = 0;
    int total = 0;
    int passed_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        // Bỏ qua khoảng trắng đầu dòng
        char *p = line;
        while (isspace((unsigned char)*p)) p++;

        // Bỏ qua dòng trống hoặc dòng chú thích 
        if (*p == '\0' || *p == '#') {
            continue;
        }

        char type;
        int n;
        uint32_t input, expected;

        // Đọc 4 trường func n input expected
        if (sscanf(p, " %c %d %x %x", &type, &n, &input, &expected) != 4) {
            printf("Dong %d sai dinh dang, bo qua: %s", line_num, line);
            continue;
        }

        type = (char)toupper((unsigned char)type);
        if ((type != 'G' && type != 'H') || n < 1 || n > 3) {
            printf("Dong %d tham so khong hop le (%c, n=%d), bo qua.\n", line_num, type, n);
            continue;
        }

        uint32_t actual = (type == 'G') ? G_box(input, n) : H_box(input, n);
        total++;

        if (actual == expected) {
            passed_count++;
            printf("[PASS] #%02d | %c_%d(0x%08X) = 0x%08X\n",
                   total, type, n, input, actual);
        } else {
            printf("[FAIL] #%02d | %c_%d(0x%08X): Thuc te = 0x%08X | Ky vong = 0x%08X\n",
                   total, type, n, input, actual, expected);
        }
    }

    fclose(fp);

    printf("Ket qua test: %d/%d test dat.\n", passed_count, total);
    return (total > 0 && passed_count == total) ? 1 : 0;
}

const uint8_t key[16] = {
        0x00, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33,
        0x44, 0x44, 0x55, 0x55, 0x66, 0x66, 0x77, 0x77
    };
const uint8_t iv[16] = {
        0x00, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33,
        0x44, 0x44, 0x55, 0x55, 0x66, 0x66, 0x77, 0x77
    };

    int test_encrypt_decrypt_roundtrip(void) {
    int res = dragon_xcrypt_file("tests/plaintext.txt","tests/ciphertext.bin" , key, iv, 128);
    printf("Ma hoa ket thuc voi trang thai: %d\n", res);
    return (res == 0) ? 1 : 0;
}

int test_f_function(const char *filepath) {
    printf("TEST dragon_F (file: %s)\n", filepath);

    FILE *fp = fopen(filepath, "r");
    if (!fp) {
        printf("Khong the mo file test: %s\n", filepath);
        return 0;
    }

    char line[512];
    int line_num = 0;
    int total = 0;
    int passed_count = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;

        char *p = line;
        while (isspace((unsigned char)*p)) p++;

        if (*p == '\0' || *p == '#') {
            continue;
        }

        uint32_t a, b, c, d, e, f;
        uint32_t exp_a, exp_b, exp_c, exp_d, exp_e, exp_f;

        if (sscanf(p, " %x %x %x %x %x %x %x %x %x %x %x %x", &a, &b, &c, &d, &e, &f, &exp_a, &exp_b, &exp_c, &exp_d, &exp_e, &exp_f) != 12) {
            printf("Dong %d sai dinh dang, bo qua: %s", line_num, line);
            continue;
        }

        // Lưu đầy đủ cả 6 giá trị đầu vào trước khi hàm dragon_F() ghi đè
        uint32_t in_a = a, in_b = b, in_c = c, in_d = d, in_e = e, in_f = f;

        dragon_F(&a, &b, &c, &d, &e, &f);
        total++;

        if (a == exp_a && b == exp_b && c == exp_c && d == exp_d && e == exp_e && f == exp_f) {
            passed_count++;
            printf("[PASS] #%02d\n", total);
            printf("Input: %08X %08X %08X %08X %08X %08X\n", in_a, in_b, in_c, in_d, in_e, in_f);
            printf("Output: %08X %08X %08X %08X %08X %08X\n", a, b, c, d, e, f);
        } else {
            printf("[FAIL] #%02d | Dong %d sai ket qua!\n", total, line_num);
            printf("Thuc te: %08X %08X %08X %08X %08X %08X\n", a, b, c, d, e, f);
            printf("Ky vong: %08X %08X %08X %08X %08X %08X\n", exp_a, exp_b, exp_c, exp_d, exp_e, exp_f);
        }
    }

    fclose(fp);
    printf("Ket qua test ham F: %d/%d test dat.\n", passed_count, total);
    return (total > 0 && passed_count == total) ? 1 : 0;
}
