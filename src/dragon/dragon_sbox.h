#ifndef DRAGON_SBOX_H
#define DRAGON_SBOX_H

#include <stdint.h>

// Khai báo mảng S1, S2 
extern const uint32_t sbox1[256];
extern const uint32_t sbox2[256];

// Khai báo các hàm phi tuyến
uint32_t dragon_G1(uint32_t x);
uint32_t dragon_G2(uint32_t x);
uint32_t dragon_H(uint32_t x);

#endif 