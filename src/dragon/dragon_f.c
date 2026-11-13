#include "dragon_f.h"

void dragon_F(uint32_t *a, uint32_t *b, uint32_t *c,
              uint32_t *d, uint32_t *e, uint32_t *f)
{
    /* Tầng 1: Trộn đầu bằng phép XOR, sau đó cộng theo modulo 2^32. */
    *b ^= *a;
    *d ^= *c;
    *f ^= *e;

    *c += *b;
    *e += *d;
    *a += *f;

    /* Tầng 2: Dùng ba biến thể G để cập nhật d, f, b. */
    *d ^= G_box(*a, 1);
    *f ^= G_box(*c, 2);
    *b ^= G_box(*e, 3);

    /* Dùng các giá trị b, d, f vừa cập nhật làm đầu vào cho ba biến thể H. */
    *a ^= H_box(*b, 1);
    *c ^= H_box(*d, 2);
    *e ^= H_box(*f, 3);

    /* Tầng 3: Trộn cuối bằng phép cộng theo modulo 2^32, sau đó XOR. */
    *d += *a;
    *f += *c;
    *b += *e;

    *c ^= *b;
    *e ^= *d;
    *a ^= *f;
}