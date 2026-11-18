#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "dragon.h"
#include "dragon_init.h"
#include "dragon_f.h"
#include "dragon_cipher.h"
#include "dragon_sbox.h"

#define CALL_INIT_KEY_IV(ctx, key, iv, bits) init_key_iv((ctx), (key), (iv), (bits))

#define CALL_XCRYPT_BUF(key, iv, bits, in, out, len) dragon_xcrypt_buffer((key), (iv), (bits), (in), (out), (len))

#define CALL_XCRYPT_FILE(in_path, out_path, key, iv, bits) dragon_xcrypt_file((in_path), (out_path), (key), (iv), (bits))

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

static void bytes_to_words(const uint8_t *bytes, uint32_t *words, size_t count) {
    for (size_t i = 0; i < count; ++i) {
        words[i] = ((uint32_t)bytes[4 * i] << 24) |
                   ((uint32_t)bytes[4 * i + 1] << 16) |
                   ((uint32_t)bytes[4 * i + 2] << 8) |
                   (uint32_t)bytes[4 * i + 3];
    }
}

static int init_key_iv(Dragon_Ctx *ctx, const uint8_t *key, const uint8_t *iv,
                       uint32_t key_bits) {
    if (!ctx || !key || !iv || (key_bits != 128 && key_bits != 256)) {
        return -1;
    }

    Dragon_Ctx key_ctx, iv_ctx;
    dragon_init_ctx(&key_ctx);
    dragon_init_ctx(&iv_ctx);
    dragon_init_ctx(ctx);

    size_t word_count = key_bits / 32;
    bytes_to_words(key, key_ctx.B, word_count);
    bytes_to_words(iv, iv_ctx.B, word_count);
    return init_key(&key_ctx, &iv_ctx, ctx, key_bits == 128 ? 1 : 0);
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

static void mo_phong_1_nhip_sinh_khoa(Dragon_Ctx *ctx,
                                      uint32_t tap_in[6],
                                      Dragon_F_Trace *f_trace,
                                      uint32_t *z0,
                                      uint32_t *z1) {
    /* Bước 1: Tăng bộ đếm trạng thái 64-bit M = (M_H || M_L) lên 1 */
    ctx->M_L++;
    if (ctx->M_L == 0) {
        ctx->M_H++;
    }

    /* Bước 2: Trích xuất 6 từ tại các vị trí tap chuẩn của Dragon */
    uint32_t a = ctx->B[0];
    uint32_t b = ctx->B[9];
    uint32_t c = ctx->B[16];
    uint32_t d = ctx->B[19];
    uint32_t e = ctx->B[30] ^ ctx->M_H;
    uint32_t f = ctx->B[31] ^ ctx->M_L;

    tap_in[0] = a; tap_in[1] = b; tap_in[2] = c;
    tap_in[3] = d; tap_in[4] = e; tap_in[5] = f;

    /* Bước 3: Đưa 6 từ qua hàm cập nhật F có lưu vết 3 tầng */
    dragon_F_trace(&a, &b, &c, &d, &e, &f, f_trace);

    /* Bước 4: Dịch thanh ghi NLFSR 1024-bit và phản hồi trạng thái mới */
    dragon_shift_nlfsr(ctx, b, c);

    /* Bước 5: Xuất 2 từ 32-bit (64-bit) khóa dòng */
    *z0 = a;
    *z1 = e;
}

static int xuat_json_mo_phong(const char *key_hex, const char *iv_hex,
                              uint32_t key_bits, int num_ticks) {
    uint8_t key[32] = {0};
    uint8_t iv[32]  = {0};
    Dragon_Ctx ctx;

    if (key_bits != 128 && key_bits != 256) {
        fprintf(stderr, "Loi: key_bits phai la 128 hoac 256.\n");
        return 1;
    }
    if (num_ticks < 1) num_ticks = 1;
    if (num_ticks > 64) num_ticks = 64;

    hex_to_bytes(key_hex, key, key_bits / 8);
    hex_to_bytes(iv_hex, iv, key_bits / 8);

    dragon_init_ctx(&ctx);
    CALL_INIT_KEY_IV(&ctx, key, iv, key_bits);

    printf("{\n");
    printf("  \"key_bits\": %u,\n", key_bits);
    printf("  \"key_hex\": \"%s\",\n", key_hex);
    printf("  \"iv_hex\": \"%s\",\n", iv_hex);

    /* Trạng thái ngay sau 16 vòng khởi tạo Key & IV */
    printf("  \"state_after_init\": {\n");
    printf("    \"M_H\": \"0x%08X\",\n", ctx.M_H);
    printf("    \"M_L\": \"0x%08X\",\n", ctx.M_L);
    printf("    \"B\": ");
    print_json_u32_array(ctx.B, 32);
    printf("\n  },\n");

    /* Danh sách từng nhịp sinh khóa dòng (Keystream Ticks) */
    printf("  \"ticks\": [\n");
    for (int t = 0; t < num_ticks; t++) {
        uint32_t tap_in[6];
        Dragon_F_Trace tr;
        uint32_t z0 = 0, z1 = 0;

        memset(&tr, 0, sizeof(tr));
        mo_phong_1_nhip_sinh_khoa(&ctx, tap_in, &tr, &z0, &z1);

        printf("    {\n");
        printf("      \"tick\": %d,\n", t + 1);
        printf("      \"M_H\": \"0x%08X\",\n", ctx.M_H);
        printf("      \"M_L\": \"0x%08X\",\n", ctx.M_L);
        printf("      \"taps_in\": ");
        print_json_u32_array(tap_in, 6);
        printf(",\n");
        printf("      \"f_trace\": {\n");
        printf("        \"in\": ");       print_json_u32_array(tr.in, 6);       printf(",\n");
        printf("        \"pre_mix\": ");  print_json_u32_array(tr.pre_mix, 6);  printf(",\n");
        printf("        \"sbox_mid\": "); print_json_u32_array(tr.sbox_layer, 6); printf(",\n");
        printf("        \"out\": ");      print_json_u32_array(tr.out, 6);      printf("\n");
        printf("      },\n");
        printf("      \"keystream_z0\": \"0x%08X\",\n", z0);
        printf("      \"keystream_z1\": \"0x%08X\",\n", z1);
        printf("      \"B_after\": ");
        print_json_u32_array(ctx.B, 32);
        printf("\n    }%s\n", (t + 1 < num_ticks) ? "," : "");
    }
    printf("  ]\n");
    printf("}\n");
    return 0;
}

static int xuat_json_ma_hoa(const char *key_hex, const char *iv_hex,
                            uint32_t key_bits, const char *plain_hex) {
    uint8_t key[32] = {0};
    uint8_t iv[32]  = {0};

    size_t plain_len = strlen(plain_hex) / 2;
    if (plain_len == 0) {
        printf("{\"error\": \"Plaintext rong\"}\n");
        return 1;
    }

    uint8_t *plain_buf  = (uint8_t *)calloc(plain_len, 1);
    uint8_t *cipher_buf = (uint8_t *)calloc(plain_len, 1);
    uint8_t *ks_buf     = (uint8_t *)calloc(plain_len, 1);
    if (!plain_buf || !cipher_buf || !ks_buf) {
        free(plain_buf); free(cipher_buf); free(ks_buf);
        return 1;
    }

    hex_to_bytes(key_hex, key, key_bits / 8);
    hex_to_bytes(iv_hex, iv, key_bits / 8);
    hex_to_bytes(plain_hex, plain_buf, plain_len);

    /* Mã hóa bản rõ để lấy bản mã */
    CALL_XCRYPT_BUF(key, iv, key_bits, plain_buf, cipher_buf, plain_len);

    /* Trích xuất chuỗi Keystream thực tế bằng phép XOR: KS = Plain ^ Cipher */
    for (size_t i = 0; i < plain_len; i++) {
        ks_buf[i] = plain_buf[i] ^ cipher_buf[i];
    }

    printf("{\n");
    printf("  \"length_bytes\": %zu,\n", plain_len);
    printf("  \"plaintext_hex\": \"");  print_bytes_hex(plain_buf, plain_len);  printf("\",\n");
    printf("  \"keystream_hex\": \"");  print_bytes_hex(ks_buf, plain_len);     printf("\",\n");
    printf("  \"ciphertext_hex\": \""); print_bytes_hex(cipher_buf, plain_len); printf("\"\n");
    printf("}\n");

    free(plain_buf);
    free(cipher_buf);
    free(ks_buf);
    return 0;
}

static void in_bang_trang_thai_terminal(const Dragon_Ctx *ctx) {
    printf("  +-------------------------------------------------------------------------+\n");
    printf("  | Bo nho trang thai 64-bit M : M_H = 0x%08X  |  M_L = 0x%08X      |\n",
           ctx->M_H, ctx->M_L);
    printf("  +-------------------------------------------------------------------------+\n");
    for (int i = 0; i < 32; i += 4) {
        printf("  | B[%02d..%02d]: 0x%08X   0x%08X   0x%08X   0x%08X          |\n",
               i, i + 3, ctx->B[i], ctx->B[i + 1], ctx->B[i + 2], ctx->B[i + 3]);
    }
    printf("  +-------------------------------------------------------------------------+\n");
}

static void chay_menu_tuong_tac(void) {
    /* Bộ khóa và IV mẫu 128-bit mặc định để test nhanh */
    uint8_t key[32] = {
        0x00, 0x00, 0x11, 0x11, 0x22, 0x22, 0x33, 0x33,
        0x44, 0x44, 0x55, 0x55, 0x66, 0x66, 0x77, 0x77
    };
    uint8_t iv[32] = {
        0x88, 0x88, 0x99, 0x99, 0xAA, 0xAA, 0xBB, 0xBB,
        0xCC, 0xCC, 0xDD, 0xDD, 0xEE, 0xEE, 0xFF, 0xFF
    };
    uint32_t key_bits = 128;
    char line[512];

    while (1) {
        printf("\n====================================================================\n");
        printf("       CHUONG TRINH MO PHONG & MA HOA HE MAT DONG DRAGON            \n");
        printf("====================================================================\n");
        printf("  Khoa hien tai (%u-bit): ", key_bits);
        print_bytes_hex(key, key_bits / 8);
        printf("\n  IV hien tai   (%u-bit): ", key_bits);
        print_bytes_hex(iv, key_bits / 8);
        printf("\n--------------------------------------------------------------------\n");
        printf("  1. Nhap Key va IV moi (dang Hex, 128-bit hoac 256-bit)\n");
        printf("  2. Ma hoa & Giai ma thu mot chuoi van ban (Plaintext)\n");
        printf("  3. Ma hoa / Giai ma file (Mac dinh: tests/plaintext.txt)\n");
        printf("  4. Mo phong tung buoc thanh ghi NLFSR B[0..31] va 3 tang ham F\n");
        printf("  0. Thoat chuong trinh\n");
        printf("--------------------------------------------------------------------\n");
        printf("Chon chuc nang (0-4): ");

        if (!fgets(line, sizeof(line), stdin)) break;
        int choice = atoi(line);

        if (choice == 0) {
            printf("Da thoat chuong trinh.\n");
            break;
        } else if (choice == 1) {
            printf("Chon do dai khoa (128 hoac 256): ");
            if (fgets(line, sizeof(line), stdin)) {
                int b = atoi(line);
                if (b == 128 || b == 256) key_bits = (uint32_t)b;
            }
            printf("Nhap Key Hex (%u ky tu hex): ", key_bits / 4);
            if (fgets(line, sizeof(line), stdin)) {
                memset(key, 0, sizeof(key));
                hex_to_bytes(line, key, key_bits / 8);
            }
            printf("Nhap IV Hex  (%u ky tu hex): ", key_bits / 4);
            if (fgets(line, sizeof(line), stdin)) {
                memset(iv, 0, sizeof(iv));
                hex_to_bytes(line, iv, key_bits / 8);
            }
            printf("[OK] Da cap nhat Key va IV!\n");

        } else if (choice == 2) {
            printf("Nhap chuoi van ban can ma hoa: ");
            if (!fgets(line, sizeof(line), stdin)) continue;
            line[strcspn(line, "\r\n")] = '\0';
            size_t len = strlen(line);
            if (len == 0) continue;

            uint8_t cipher[512]    = {0};
            uint8_t decrypted[512] = {0};

            CALL_XCRYPT_BUF(key, iv, key_bits, (const uint8_t *)line, cipher, len);
            CALL_XCRYPT_BUF(key, iv, key_bits, cipher, decrypted, len);
            decrypted[len] = '\0';

            printf("\n--- KET QUA MA HOA & GIAI MA ---\n");
            printf("  Ban ro (Text)      : %s (%zu bytes)\n", line, len);
            printf("  Ban ro (Hex)       : "); print_bytes_hex((const uint8_t *)line, len); printf("\n");
            printf("  Khoa dong (Hex)    : ");
            for (size_t i = 0; i < len; i++) printf("%02X", ((uint8_t)line[i]) ^ cipher[i]);
            printf("\n");
            printf("  Ban ma (Hex)       : "); print_bytes_hex(cipher, len); printf("\n");
            printf("  Giai ma nguoc (Text): %s\n", (char *)decrypted);

        } else if (choice == 3) {
            char in_file[256]  = "tests/plaintext.txt";
            char out_file[256] = "tests/ciphertext.bin";

            printf("Nhap duong dan file dau vao [Enter = tests/plaintext.txt]: ");
            if (fgets(line, sizeof(line), stdin)) {
                line[strcspn(line, "\r\n")] = '\0';
                if (strlen(line) > 0) {
                    size_t path_len = strlen(line);
                    if (path_len >= sizeof(in_file)) path_len = sizeof(in_file) - 1;
                    memcpy(in_file, line, path_len);
                    in_file[path_len] = '\0';
                }
            }
            printf("Nhap duong dan file dau ra  [Enter = tests/ciphertext.bin]: ");
            if (fgets(line, sizeof(line), stdin)) {
                line[strcspn(line, "\r\n")] = '\0';
                if (strlen(line) > 0) {
                    size_t path_len = strlen(line);
                    if (path_len >= sizeof(out_file)) path_len = sizeof(out_file) - 1;
                    memcpy(out_file, line, path_len);
                    out_file[path_len] = '\0';
                }
            }

            if (CALL_XCRYPT_FILE(in_file, out_file, key, iv, key_bits) == 0) {
                printf("[OK] Da xu ly thanh cong: %s -> %s\n", in_file, out_file);
            } else {
                printf("[LOI] Khong the mo hoac xu ly file!\n");
            }

        } else if (choice == 4) {
            Dragon_Ctx ctx;
            dragon_init_ctx(&ctx);
            CALL_INIT_KEY_IV(&ctx, key, iv, key_bits);

            printf("\n[TRANG THAI SAU 16 VONG KHOI TAO KEY & IV]\n");
            in_bang_trang_thai_terminal(&ctx);

            printf("Nhap so nhip sinh khoa dong muon theo doi (vi du 2): ");
            int ticks = 1;
            if (fgets(line, sizeof(line), stdin)) {
                ticks = atoi(line);
                if (ticks < 1) ticks = 1;
                if (ticks > 10) ticks = 10;
            }

            for (int t = 1; t <= ticks; t++) {
                uint32_t tap_in[6], z0 = 0, z1 = 0;
                Dragon_F_Trace tr;
                memset(&tr, 0, sizeof(tr));

                mo_phong_1_nhip_sinh_khoa(&ctx, tap_in, &tr, &z0, &z1);

                printf("\n>>> NHIP SINH KHOA THU %d <<<\n", t);
                printf("  1. Dau vao ham F (a..f) : %08X %08X %08X %08X %08X %08X\n",
                       tr.in[0], tr.in[1], tr.in[2], tr.in[3], tr.in[4], tr.in[5]);
                printf("  2. Sau tang Pre-mixing  : %08X %08X %08X %08X %08X %08X\n",
                       tr.pre_mix[0], tr.pre_mix[1], tr.pre_mix[2],
                       tr.pre_mix[3], tr.pre_mix[4], tr.pre_mix[5]);
                printf("  3. Sau tang S-box (G,H) : %08X %08X %08X %08X %08X %08X\n",
                       tr.sbox_layer[0], tr.sbox_layer[1], tr.sbox_layer[2],
                       tr.sbox_layer[3], tr.sbox_layer[4], tr.sbox_layer[5]);
                printf("  4. Dau ra Post-mixing   : %08X %08X %08X %08X %08X %08X\n",
                       tr.out[0], tr.out[1], tr.out[2], tr.out[3], tr.out[4], tr.out[5]);
                printf("  => KHOA DONG XUAT RA (64-bit): z0 = 0x%08X | z1 = 0x%08X\n", z0, z1);
                in_bang_trang_thai_terminal(&ctx);
            }
        }
    }
}


int main(int argc, char *argv[]) {
    /* Nếu chạy không có tham số -> Mở menu tương tác trên Terminal */
    if (argc == 1) {
        chay_menu_tuong_tac();
        return 0;
    }

    /* Chế độ 1: Xuất JSON mô phỏng từng bước cho Tab 1 của GUI (TV8)
     * Cú pháp: ./dragon_cli --trace-json <key_hex> <iv_hex> <key_bits> <num_ticks> */
    if (argc >= 6 && strcmp(argv[1], "--trace-json") == 0) {
        uint32_t key_bits = (uint32_t)atoi(argv[4]);
        int num_ticks     = atoi(argv[5]);
        return xuat_json_mo_phong(argv[2], argv[3], key_bits, num_ticks);
    }

    /* Chế độ 2: Xuất JSON mã hóa trực quan cho Tab 2 của GUI (TV9)
     * Cú pháp: ./dragon_cli --encrypt-hex <key_hex> <iv_hex> <key_bits> <plain_hex> */
    if (argc >= 6 && strcmp(argv[1], "--encrypt-hex") == 0) {
        uint32_t key_bits = (uint32_t)atoi(argv[4]);
        return xuat_json_ma_hoa(argv[2], argv[3], key_bits, argv[5]);
    }

    /* Chế độ 3: Mã hóa / Giải mã file qua dòng lệnh cho GUI
     * Cú pháp: ./dragon_cli --crypt-file <key_hex> <iv_hex> <key_bits> <in_file> <out_file> */
    if (argc >= 7 && strcmp(argv[1], "--crypt-file") == 0) {
        uint8_t key[32] = {0}, iv[32] = {0};
        uint32_t key_bits = (uint32_t)atoi(argv[4]);
        hex_to_bytes(argv[2], key, key_bits / 8);
        hex_to_bytes(argv[3], iv, key_bits / 8);
        return CALL_XCRYPT_FILE(argv[5], argv[6], key, iv, key_bits);
    }

    printf("Huong dan su dung Dragon CLI:\n");
    printf("  1. Chay menu tuong tac : %s\n", argv[0]);
    printf("  2. Xuat JSON mo phong  : %s --trace-json  <key_hex> <iv_hex> <128|256> <ticks>\n", argv[0]);
    printf("  3. Xuat JSON ma hoa    : %s --encrypt-hex <key_hex> <iv_hex> <128|256> <plain_hex>\n", argv[0]);
    printf("  4. Ma hoa/Giai ma file : %s --crypt-file  <key_hex> <iv_hex> <128|256> <in> <out>\n", argv[0]);
    return 0;
}
