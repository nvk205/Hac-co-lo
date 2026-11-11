#ifndef CRYPTO_EVAL_H
#define CRYPTO_EVAL_H

#include <stdio.h>
#include "dragon.h"

/* Số vòng khởi tạo theo mật mã Dragon */
#define EVAL_INIT_ROUNDS 16

/* Trạng thái: gồm 32 từ B và bộ đếm 64 bit M */
typedef struct {
    u_int B[32];
    u64   M;
} eval_state;

/* Kiểm tra bản cài đặt có khớp với lõi Dragon và test vector
 * Trả về 0 nếu khớp, khác 0 nếu không khớp */
int eval_selfcheck(void);

/* Thí nghiệm 1a: so sánh 2 khóa khác nhau 1 bit */
void eval_exp1_single(int kind, FILE *csv);

/* Thí nghiệm 1b: Lắp thí nghiệm avalanche với các cặp khóa/IV ngẫu nhiên */
void eval_exp1_stats(int kind, int trials, FILE *csv_rounds, FILE *csv_summary);

/* Thí nghiệm 2: Thống kê keystream 1 MB trên num_keys khóa ngẫu nhiên */
void eval_exp2(int num_keys, FILE *csv_bytes, FILE *csv_many);

/* Thí nghiệm 3: Minh họa tấn công dùng lại IV */
void eval_exp3(void);

/* Tính p-value cho kiểm định tàn suất bit và kiểm định chi bình phương */
double eval_erfc_p_monobit(unsigned long long ones, unsigned long long nbits);
double eval_chi2_pvalue(double chi2, int dof);

#endif /* CRYPTO_EVAL_H */