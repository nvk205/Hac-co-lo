#ifndef DRAGON_H
#define DRAGON_H

#include <stdint.h>

extern const uint32_t S1[256];
extern const uint32_t S2[256];

//Function goi cac ham Gi
uint32_t G1(uint32_t x);
uint32_t G2(uint32_t x);
uint32_t G3(uint32_t x);

//Function goi cac ham Hi
uint32_t H1(uint32_t x);
uint32_t H2(uint32_t x);
uint32_t H3(uint32_t x);

#endif