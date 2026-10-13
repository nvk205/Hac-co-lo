#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include "dragon.h"

#define MAX_LEN 1024

static void print_hex(const uint8_t *d, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        printf("%02X", d[i]);
        if ((i + 1) % 8 == 0 && i + 1 < n) putchar(' ');
    }
    putchar('\n');
}

/* Nhap Plain text & Giai ma */
static int encrypt_demo(void)
{
    u64 key[2] = { 0x0000111122223333ULL, 0x4444555566667777ULL };
    u64 iv[2]  = { 0x0000111122223333ULL, 0x4444555566667777ULL };

    char    line[MAX_LEN + 2];
    uint8_t plain[MAX_LEN], cipher[MAX_LEN], decoded[MAX_LEN + 1];
    u_int   B[32];
    size_t  len;

    printf("Nhap Plain text      : ");
    fflush(stdout);
    if (!fgets(line, sizeof line, stdin)) {
        fprintf(stderr, "Khong doc duoc du lieu nhap.\n");
        return 1;
    }
    if (!strchr(line, '\n') && strlen(line) == sizeof line - 1) {
        fprintf(stderr, "Plain text qua dai (toi da %d byte).\n", MAX_LEN);
        return 1;
    }
    len = strcspn(line, "\r\n");
    if (len == 0) {
        fprintf(stderr, "Plain text rong.\n");
        return 1;
    }
    memcpy(plain, line, len);
    dragon_state_init(B, key, iv, DRAGON_128);
    dragon_crypt(B, plain, cipher, len);

    printf("Do dai Plain text    : %zu byte\n", len);
    printf("Ban ma (hex)         : ");
    print_hex(cipher, len);

    dragon_state_init(B, key, iv, DRAGON_128);
    dragon_crypt(B, cipher, decoded, len);
    decoded[len] = '\0';

    printf("Giai ma              : %s\n", (char *)decoded);

    if (memcmp(plain, decoded, len) != 0) {
        fprintf(stderr, "LOI: giai ma khong khop Plain text.\n");
        return 2;
    }
    return 0;
}

int main(void)
{
    u64 key[2] = {
        0x0000111122223333ULL,
        0x4444555566667777ULL
    };
    u64 iv[2] = {
        0x0000111122223333ULL,
        0x4444555566667777ULL
    };

    element W[8];
    u_int B[32];
    u64 z;
    int j, k;

    init_key(key, iv, W, 1);

    for (j = 0, k = 0; k < 8; k++, j += 4) {
        B[j] = (u_int)(W[k].lb >> 32);
        B[j + 1] = (u_int)W[k].lb;
        B[j + 2] = (u_int)(W[k].rb >> 32);
        B[j + 3] = (u_int)W[k].rb;
    }

    for (int x = 0; x < 16; x++) {
        gen_key(B, &z);
        printf("%016" PRIx64 "\n", z);
    }
    
    printf("\n");
    encrypt_demo();

    return 0;
}