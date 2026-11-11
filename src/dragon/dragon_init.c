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
	
}
