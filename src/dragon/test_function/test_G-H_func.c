#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include "dragon_sbox.h"

int main() {
    // Ví dụ chạy các hàm G_box và H_box
    u_int x = 0x12345678;
    int n = 1;

    u_int g_result = G_box(x, n);
    u_int h_result = H_box(x, n);

    printf("G_box result: 0x%X\n", g_result);
    printf("H_box result: 0x%X\n", h_result);

    return 0;
}
/* Giải thích:
x = 0x12345678 = 00010010 00110100 01010110 01111000

B1: Tách x thành 4 byte x_0, x_1, x_2, x_3
x_0 = x >> 24 = 00000000 00000000 00000000 00010010 = 0x12 & 0xFF = 0x12
x_1 = x >> 16 = 00000000 00000000 00010010 00110100 = 0x1234 & 0xFF = 0x34
x_2 = x >> 8  = 00000000 00010010 00110100 01010110 = 0x123456 & 0xFF = 0x56
x_3 = x       = 00010010 00110100 01010110 01111000 = 0x12345678 & 0xFF = 0x78

B2: Tra cứu trong bảng S_box1 và S_box2 với n = 1
G_box: sbox1[0x12] ^ sbox1[0x34] ^ sbox1[0x56] ^ sbox2[0x78]
H_box: sbox2[0x12] ^ sbox2[0x34] ^ sbox2[0x56] ^ sbox1[0x78]

B3: Thực hiện phép XOR các giá trị tra cứu được từ các byte
g_result = sbox1[0x12] ^ sbox1[0x34] ^ sbox1[0x56] ^ sbox2[0x78] 
         = 0x1D89BF75 ^ 0x9711B3F0 ^ 0xD8A0FA28 ^ 0x8B937233 
         = 0xD9AB849E

h_result = sbox2[0x12] ^ sbox2[0x34] ^ sbox2[0x56] ^ sbox1[0x78] 
         = 0x0CBF1D6F ^ 0xEDB7705D ^ 0x8B6AD73E ^ 0xB7C3B0FF 
         = 0xDDA10AF3
*/