#define __STDC_FORMAT_MACROS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "dragon.h"

int run_dragon_test(void) {
    FILE *f = fopen("vectors.txt", "r");
    if (!f) {
        printf("[LOI] Khong the mo file vectors.txt!\n");
        printf("dam bao 'vectors.txt' trong thu muc voi project.\n\n");
        return -1;
    }

    char tag[10];
    int bits = 128;
    char hex_str[512];
    u64 key[4] = {0};
    u64 iv[4] = {0};
    u64 expected_z[16] = {0};

    u_int B[32];
    u64 z;
    int test_count = 0;
    int total_fails = 0;

    while (fscanf(f, "%s", tag) != EOF) {
        if (tag[0] == '#') {
            fgets(hex_str, sizeof(hex_str), f);
            continue;
        }

        /* Đọc KEY */
        if (strcmp(tag, "KEY") == 0) {
            fscanf(f, "%d %s", &bits, hex_str);
            int words = (bits == 128) ? 2 : 4;
            for (int i = 0; i < words; i++) {
                char buf[17] = {0};
                strncpy(buf, hex_str + i * 16, 16);
                key[i] = strtoull(buf, NULL, 16);
            }
        } 
        /* 2. Đọc IV */
        else if (strcmp(tag, "IV") == 0) {
            fscanf(f, "%d %s", &bits, hex_str);
            int words = (bits == 128) ? 2 : 4;
            for (int i = 0; i < words; i++) {
                char buf[17] = {0};
                strncpy(buf, hex_str + i * 16, 16);
                iv[i] = strtoull(buf, NULL, 16);
            }
        } 
        /* Đọc KEYSTREAM */
        else if (strcmp(tag, "KS") == 0) {
            fscanf(f, "%s", hex_str);
            for (int i = 0; i < 16; i++) {
                char buf[17] = {0};
                strncpy(buf, hex_str + i * 16, 16);
                expected_z[i] = strtoull(buf, NULL, 16);
            }

            /* Test Vector */
            test_count++;
            int set_fails = 0;
            int kind = (bits == 128) ? DRAGON_128 : DRAGON_256;

            printf("----------------------------------------------------------------------\n");
            printf("TEST VECTOR %d (%d-bit Key/IV)\n", test_count, bits);
            printf("----------------------------------------------------------------------\n");

            dragon_state_init(B, key, iv, kind);

            /* In kết quả */
            printf("%-4s | %-16s | %-16s | %s\n", "STT", "Thuc te (Actual)", "Ky vong (Expected)", "Trang thai");
            printf("-----+------------------+------------------+------------\n");

            for (int i = 0; i < 16; i++) {
                gen_key(B, &z);
                int is_match = (z == expected_z[i]);
                if (!is_match) set_fails++;

                printf("%02d   | %016" PRIx64 " | %016" PRIx64 " | [%s]\n", 
                       i + 1, z, expected_z[i], is_match ? "PASS" : "FAIL");
            }

            printf("\nTEST %d: %s (%d/16)\n\n", 
                   test_count, (set_fails == 0) ? "DAT (PASS)" : "KHONG DAT (FAIL)", 16 - set_fails);

            total_fails += set_fails;
        }
    }

    fclose(f);

    return total_fails;
}