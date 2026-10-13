#ifndef DRAGON_H
#define DRAGON_H

#include <stdint.h>
#include <stddef.h>

typedef uint64_t u64;
typedef unsigned int u_int;

typedef struct {
    u64 lb;
    u64 rb;
} element;

extern unsigned int sbox1[256];
extern unsigned int sbox2[256];

int F_func(u_int *A);
u_int G_box(u_int x, int n);
u_int H_box(u_int x, int n);
int init_key(u64 *key, u64 *iv, element *W, int n);
int gen_key(u_int *B, u64 *z);

#define DRAGON_128 1  
#define DRAGON_256 2  

void dragon_state_init(u_int *B, u64 *key, u64 *iv, int kind);

void dragon_crypt(u_int *B, const uint8_t *in, uint8_t *out, size_t len);

int dragon_selftest(int verbose);

#endif