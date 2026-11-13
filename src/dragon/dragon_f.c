#include "dragon_f.h"
#include <stddef.h>
/* Ghi lại sáu từ trạng thái theo thứ tự a, b, c, d, e, f. */
static void luu_trang_thai(uint32_t dich[6],
                           uint32_t a, uint32_t b, uint32_t c,
                           uint32_t d, uint32_t e, uint32_t f)
{
    dich[0] = a;
    dich[1] = b;
    dich[2] = c;
    dich[3] = d;
    dich[4] = e;
    dich[5] = f;
}
void dragon_F_trace(uint32_t *a, uint32_t *b, uint32_t *c,
                    uint32_t *d, uint32_t *e, uint32_t *f,
                    Dragon_F_Trace *trace)
{
    if (trace != NULL) { luu_trang_thai(trace->in, *a, *b, *c, *d, *e, *f); }
    /* Tầng 1: Trộn đầu bằng phép XOR, sau đó cộng theo modulo 2^32. */
    *b ^= *a;
    *d ^= *c;
    *f ^= *e;

    *c += *b;
    *e += *d;
    *a += *f;
	if (trace != NULL) { luu_trang_thai(trace->pre_mix, *a, *b, *c, *d, *e, *f); }
    /* Tầng 2: Dùng ba biến thể G để cập nhật d, f, b. */
    *d ^= G_box(*a, 1);
    *f ^= G_box(*c, 2);
    *b ^= G_box(*e, 3);

    /* Dùng các giá trị b, d, f vừa cập nhật làm đầu vào cho ba biến thể H. */
    *a ^= H_box(*b, 1);
    *c ^= H_box(*d, 2);
    *e ^= H_box(*f, 3);
	if (trace != NULL) { luu_trang_thai(trace->sbox_layer, *a, *b, *c, *d, *e, *f); }
    /* Tầng 3: Trộn cuối bằng phép cộng theo modulo 2^32, sau đó XOR. */
    *d += *a;
    *f += *c;
    *b += *e;

    *c ^= *b;
    *e ^= *d;
    *a ^= *f;
    if (trace != NULL) { luu_trang_thai(trace->out, *a, *b, *c, *d, *e, *f); }
}
void dragon_F(uint32_t *a, uint32_t *b, uint32_t *c,
              uint32_t *d, uint32_t *e, uint32_t *f)
{
    /* Dùng chung ba tầng tính toán và bỏ qua phần ghi trạng thái. */
    dragon_F_trace(a, b, c, d, e, f, NULL);
}