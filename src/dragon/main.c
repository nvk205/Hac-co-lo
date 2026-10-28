
#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdint.h>
#include "dragon.h"

#define MAX_LEN 1024

/* In du lieu dang hexadecimal */
static void print_hex(const uint8_t *data, size_t len)
{
    for (size_t i = 0; i < len; i++) {
        printf("%02X", (unsigned int)data[i]);

        if ((i + 1) % 8 == 0 && i + 1 < len) {
            putchar(' ');
        }
    }

    putchar('\n');
}

/* Nhap Plain text, ma hoa va giai ma */
static int encrypt_demo(void)
{
    u64 key[2] = {
        0x0000111122223333ULL,
        0x4444555566667777ULL
    };

    u64 iv[2] = {
        0x0000111122223333ULL,
        0x4444555566667777ULL
    };

    char line[MAX_LEN + 2];
    uint8_t plain[MAX_LEN];
    uint8_t cipher[MAX_LEN];
    uint8_t decoded[MAX_LEN + 1];

    u_int B[32];
    size_t len;

    printf("Nhap Plain text      : ");
    fflush(stdout);

    if (fgets(line, sizeof(line), stdin) == NULL) {
        fprintf(stderr, "Khong doc duoc du lieu nhap.\n");
        return 1;
    }

    /*
     * Neu khong doc duoc ky tu xuong dong va bo dem da day,
     * kiem tra xem du lieu co vuot qua gioi han hay khong.
     */
    if (strchr(line, '\n') == NULL &&
        strchr(line, '\r') == NULL &&
        strlen(line) == sizeof(line) - 1) {
        fprintf(stderr, "Plain text qua dai (toi da %d byte).\n",
                MAX_LEN);
        return 1;
    }

    len = strcspn(line, "\r\n");

    if (len == 0) {
        fprintf(stderr, "Plain text rong.\n");
        return 1;
    }

    if (len > MAX_LEN) {
        fprintf(stderr, "Plain text qua dai (toi da %d byte).\n",
                MAX_LEN);
        return 1;
    }

    memcpy(plain, line, len);

    /* Ma hoa */
    dragon_state_init(B, key, iv, DRAGON_128);
    dragon_crypt(B, plain, cipher, len);

    printf("\nDo dai Plain text    : %lu byte\n",
           (unsigned long)len);

    printf("Plain text (hex)     : ");
    print_hex(plain, len);

    printf("Ban ma (hex)         : ");
    print_hex(cipher, len);

    /* Giai ma: khoi tao lai trang thai voi cung key va IV */
    dragon_state_init(B, key, iv, DRAGON_128);
    dragon_crypt(B, cipher, decoded, len);

    decoded[len] = '\0';

    printf("Giai ma              : %s\n", (char *)decoded);

    /* Kiem tra ket qua giai ma */
    if (memcmp(plain, decoded, len) != 0) {
        fprintf(stderr,
                "LOI: Giai ma khong khop voi Plain text ban dau.\n");
        return 2;
    }

    printf("Kiem tra round-trip  : PASS\n");

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

    int j;
    int k;

    /* Khoi tao khoa va trang thai ban dau */
    if (init_key(key, iv, W, 1) != 0) {
        fprintf(stderr, "Loi khi khoi tao khoa Dragon.\n");
        return 1;
    }

    for (j = 0, k = 0; k < 8; k++, j += 4) {
        B[j]     = (u_int)(W[k].lb >> 32);
        B[j + 1] = (u_int)W[k].lb;
        B[j + 2] = (u_int)(W[k].rb >> 32);
        B[j + 3] = (u_int)W[k].rb;
    }

    /* In 16 gia tri keystream */
    printf("DRAGON KEYSTREAM \n");

    for (int x = 0; x < 16; x++) {
        if (gen_key(B, &z) != 0) {
            fprintf(stderr, "Loi khi sinh keystream.\n");
            return 1;
        }

        printf("*%016" PRIx64 "\n", (uint64_t)z);
    }

    printf("\n ENCRYPTION / DECRYPTION \n");

    return encrypt_demo();
}
