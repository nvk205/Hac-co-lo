#include "dragon_init.h"


//n để chọn mode: n=1 là 128 bit, n=0 là 256 bit
int init_key(Dragon_Ctx *key, Dragon_Ctx *iv, Dragon_Ctx *W, int n)
{
    if (n==1){  //128 bit
	/*Mỗi W, key và iv có 128 bít, 
	mà Khánh bắt dùng kiểu dữ liệu 32 bit, nên mỗi cái gồm 4 phần tử 32 bit của mảng B
	k là A|B, k' là B|A
	iv là C|D, iv' là D|C
	*/
		W->B[0] = key->B[0];	//W[0]
		W->B[1] = key->B[1];
		W->B[2] = key->B[2];
		W->B[3] = key->B[3];
		W->B[4] = key->B[2] ^ iv->B[2];	//W[1], (k' XOR iv') 
		W->B[5] = key->B[3] ^ iv->B[3];
		W->B[6] = key->B[0] ^ iv->B[0];
		W->B[7] = key->B[1] ^ iv->B[1];
		W->B[8] = iv->B[0];				//W[2], iv
		W->B[9] = iv->B[1];
		W->B[10] = iv->B[2];
		W->B[11] = iv->B[3];
		W->B[12] = key->B[0] ^ iv->B[2];	//W[3], k XOR iv'
		W->B[13] = key->B[1] ^ iv->B[3];
		W->B[14] = key->B[2] ^ iv->B[0];
		W->B[15] = key->B[3] ^ iv->B[1];
		W->B[16] = key->B[2];				//W[4], k'
		W->B[17] = key->B[3];
		W->B[18] = key->B[0];
		W->B[19] = key->B[1];
		W->B[20] = key->B[0] ^ iv->B[0];	//W[5], k XOR iv
		W->B[21] = key->B[1] ^ iv->B[1];
		W->B[22] = key->B[2] ^ iv->B[2];
		W->B[23] = key->B[3] ^ iv->B[3];
		W->B[24] = iv->B[2];				//W[6], iv'
		W->B[25] = iv->B[3];
		W->B[26] = iv->B[0];
		W->B[27] = iv->B[1];
		W->B[28] = key->B[2] ^ iv->B[0];	//W[7], k' XOR iv
		W->B[29] = key->B[3] ^ iv->B[1];
		W->B[30] = key->B[0] ^ iv->B[2];
		W->B[31] = key->B[1] ^ iv->B[3];
	} else {    //256 bit
		/*
		W, key, iv 256 bit, mỗi cái gồm 8 phần tử 32 bit của mảng B
		*/
	//W[1]|W[0] = K
		W->B[0] = key->B[0];
		W->B[1] = key->B[1];
		W->B[2] = key->B[2];
		W->B[3] = key->B[3];
		W->B[4] = key->B[4];
		W->B[5] = key->B[5];
		W->B[6] = key->B[6];
		W->B[7] = key->B[7];
	//W[3]|W[2] = K XOR IV
		W->B[8] = key->B[0] ^ iv->B[0];
		W->B[9] = key->B[1] ^ iv->B[1];
		W->B[10] = key->B[2] ^ iv->B[2];
		W->B[11] = key->B[3] ^ iv->B[3];
		W->B[12] = key->B[4] ^ iv->B[4];
		W->B[13] = key->B[5] ^ iv->B[5];
		W->B[14] = key->B[6] ^ iv->B[6];
		W->B[15] = key->B[7] ^ iv->B[7];
	//W[5]|W[4] = ~(K XOR IV)
		W->B[16] = ~(key->B[0] ^ iv->B[0]);
		W->B[17] = ~(key->B[1] ^ iv->B[1]);
		W->B[18] = ~(key->B[2] ^ iv->B[2]);
		W->B[19] = ~(key->B[3] ^ iv->B[3]);
		W->B[20] = ~(key->B[4] ^ iv->B[4]);
		W->B[21] = ~(key->B[5] ^ iv->B[5]);
		W->B[22] = ~(key->B[6] ^ iv->B[6]);
		W->B[23] = ~(key->B[7] ^ iv->B[7]);
	//W[7]|W[6] = IV
		W->B[24] = iv->B[0];
		W->B[25] = iv->B[1];
		W->B[26] = iv->B[2];
		W->B[27] = iv->B[3];
		W->B[28] = iv->B[4];
		W->B[29] = iv->B[5];
		W->B[30] = iv->B[6];
		W->B[31] = iv->B[7];
	}
	
	uint32_t A[6];
	uint32_t new_B[4];
	for (int i = 0; i <16; i++){
	/*
	Mảng A gồm  phần tử a,b,c,d,e,f
	Trong đó: a||b||c||d = (W0 XOR W6 XOR W7)
				e||f = M
	*/
	A[0] = W->B[0] ^ W->B[24] ^ W->B[28];
    A[1] = W->B[1] ^ W->B[25] ^ W->B[29];
    A[2] = W->B[2] ^ W->B[26] ^ W->B[30];
    A[3] = W->B[3] ^ W->B[27] ^ W->B[31];
	A[4] = W->M_L;
    A[5] = W->M_H;

	//{a',b',c',d'} = F(a,b,c,d,e,f)
	F_func(A);
	//W0 = (a'||b'||c'||d') XOR W4
	W->B[0] = A[0] ^ W->B[16];
	W->B[1] = A[1] ^ W->B[17];
	W->B[2] = A[2] ^ W->B[18];
	W->B[3] = A[3] ^ W->B[19];
	//Wi =Wi-1 for i = 7 to 1
	for (int j = 31; j >=4; j--){
		W->B[j] = W->B[j-4];
	}
	//M = e'||f'
	W->M_L = A[4];
	W->M_H = A[5];
}
	return 0;
}

