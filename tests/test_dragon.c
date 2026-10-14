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
