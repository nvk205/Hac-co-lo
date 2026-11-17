#ifndef DRAGON_SBOX_H
#define DRAGON_SBOX_H

#include <stdint.h>
#include <sys/types.h>

#ifndef u_int
typedef unsigned int u_int;
#endif

// Khai báo các bảng tra cứu tĩnh S_box
extern const unsigned int sbox1[256];
extern const unsigned int sbox2[256];

//Khai báo các hàm biến đổi phi tuyến
u_int G_box(u_int x, int n);
u_int H_box(u_int x, int n);

#endif